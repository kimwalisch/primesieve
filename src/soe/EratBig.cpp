///
/// @file   EratBig.cpp
/// @brief  Segmented sieve of Eratosthenes optimized for big sieving
///         primes. This is an optimized implementation of Tomas
///         Oliveira e Silva's cache-friendly bucket sieve algorithm:
///         http://www.ieeta.pt/~tos/software/prime_sieve.html
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the New BSD License. See the
/// LICENSE file in the top level directory.
///

#include "config.h"
#include "EratBig.h"
#include "WheelFactorization.h"
#include "Bucket.h"
#include "primesieve_error.h"
#include "imath.h"

#include <stdint.h>
#include <cstdlib>
#include <cassert>
#include <algorithm>
#include <vector>
#include <list>

namespace soe {

/// @param stop       Upper bound for sieving.
/// @param sieveSize  Sieve size in bytes.
/// @param limit      Sieving primes in EratBig must be <= limit,
///                   usually limit = sqrt(stop).
///
EratBig::EratBig(uint64_t stop, uint_t sieveSize, uint_t limit) :
  Modulo210Wheel_t(stop, sieveSize),
  limit_(limit),
  log2SieveSize_(ilog2(sieveSize)),
  moduloSieveSize_(sieveSize - 1),
  stock_(NULL)
{
  // '>> log2SieveSize' requires a power of 2 sieveSize
  if (!isPowerOf2(sieveSize))
    throw primesieve_error("EratBig: sieveSize must be a power of 2");
  init(sieveSize);
}

EratBig::~EratBig()
{
  for (PointerIterator_t iter = pointers_.begin(); iter != pointers_.end(); ++iter)
    delete[] *iter;
}

void EratBig::init(uint_t sieveSize)
{
  uint_t maxSievingPrime  = limit_ / NUMBERS_PER_BYTE;
  uint_t maxNextMultiple  = maxSievingPrime * getMaxFactor() + getMaxFactor();
  uint_t maxMultipleIndex = sieveSize - 1 + maxNextMultiple;
  uint_t maxSegmentCount  = maxMultipleIndex >> log2SieveSize_;
  lists_.resize(maxSegmentCount + 1, NULL);
  bucketCache_.resize(lists_.size());
  for (uint_t i = 0; i < lists_.size(); i++)
    pushBucket(i);
}

/// Get the segment corresponding to the next multiple
/// (multipleIndex) of a sievingPrime.
///
uint_t EratBig::getSegment(uint_t* multipleIndex)
{
  uint_t segment = *multipleIndex >> log2SieveSize_;
  *multipleIndex &= moduloSieveSize_;
  return segment;
}

/// Store a new sieving prime in EratBig
void EratBig::store(uint_t prime, uint_t multipleIndex, uint_t wheelIndex)
{
  assert(prime <= limit_);
  uint_t sievingPrime = prime / NUMBERS_PER_BYTE;
  uint_t segment = getSegment(&multipleIndex);
  if (!bucketCache_[segment].store(sievingPrime, multipleIndex, wheelIndex))
    pushBucket(segment);
}

void EratBig::moveBucket(Bucket& src, Bucket*& dest)
{
  src.setNext(dest);
  dest = &src;
}

/// Add an empty bucket to the list corresponding to the
/// segment parameter i.e. lists_[segment].
///
void EratBig::pushBucket(uint_t segment)
{
  // if the stock_ is empty allocate new buckets
  if (!stock_) {
    const int N = config::MEMORY_PER_ALLOC / sizeof(Bucket);
    Bucket* buckets = new Bucket[N];
    for(int i = 0; i < N-1; i++)
      buckets[i].setNext(&buckets[i + 1]);
    buckets[N-1].setNext(NULL);
    pointers_.push_front(buckets);
    stock_ = buckets;
  }
  Bucket* bucket = stock_;
  stock_ = stock_->next();
  updateCache(*bucket, segment);
  moveBucket(*bucket, lists_[segment]);
}

void EratBig::updateCache(Bucket& bucket, uint_t segment)
{
  if (lists_[segment])
    lists_[segment]->setFull();
  bucketCache_[segment].set(bucket);
}

void EratBig::cacheWriteBack()
{
  lists_[0]->update(bucketCache_[0]);
}

void EratBig::rotate()
{
  // move the list corresponding to the next segment
  // i.e. lists_[1] to lists_[0] ...
  std::rotate(lists_.begin(),
              lists_.begin() + 1,
              lists_.end());

  std::rotate(bucketCache_.begin(),
              bucketCache_.begin() + 1,
              bucketCache_.end());
}

/// Cross-off the multiples of big sieving primes
/// from the sieve array.
///
void EratBig::crossOff(byte_t* sieve)
{
  cacheWriteBack();

  // process the buckets in lists_[0] which hold the sieving primes
  // that have multiple(s) in the current segment
  while (lists_[0]->hasNext() || !lists_[0]->empty()) {
    Bucket* bucket = lists_[0];
    lists_[0] = NULL;
    pushBucket(0);
    do {
      crossOff(sieve, *bucket);
      Bucket* processed = bucket;
      bucket = bucket->next();
      processed->reset();
      moveBucket(*processed, stock_);
    } while (bucket);

    cacheWriteBack();
  }
  rotate();
}

/// Cross-off the next multiple of each sieving prime within the
/// current bucket. This is an implementation of the segmented sieve
/// of Eratosthenes with wheel factorization optimized for big sieving
/// primes that have very few multiples per segment. This algorithm
/// uses a modulo 210 wheel that skips multiples of 2, 3, 5 and 7.
///
void EratBig::crossOff(byte_t* sieve, Bucket& bucket)
{
  WheelPrime* wPrime = bucket.begin();
  WheelPrime* end    = bucket.end();

  // 2 sieving primes are processed per loop iteration
  // to increase instruction level parallelism
  for (; wPrime + 2 <= end; wPrime += 2) {
    uint_t multipleIndex0 = wPrime[0].getMultipleIndex();
    uint_t wheelIndex0    = wPrime[0].getWheelIndex();
    uint_t sievingPrime0  = wPrime[0].getSievingPrime();
    uint_t multipleIndex1 = wPrime[1].getMultipleIndex();
    uint_t wheelIndex1    = wPrime[1].getWheelIndex();
    uint_t sievingPrime1  = wPrime[1].getSievingPrime();
    // cross-off the current multiple (unset bit)
    // and calculate the next multiple
    unsetBit(sieve, sievingPrime0, &multipleIndex0, &wheelIndex0);
    unsetBit(sieve, sievingPrime1, &multipleIndex1, &wheelIndex1);
    uint_t segment0 = getSegment(&multipleIndex0);
    uint_t segment1 = getSegment(&multipleIndex1);
    // move the 2 sieving primes to the list related
    // to their next multiple
    if (!bucketCache_[segment0].store(sievingPrime0, multipleIndex0, wheelIndex0))
      pushBucket(segment0);
    if (!bucketCache_[segment1].store(sievingPrime1, multipleIndex1, wheelIndex1))
      pushBucket(segment1);
  }

  if (wPrime != end) {
    uint_t multipleIndex = wPrime->getMultipleIndex();
    uint_t wheelIndex    = wPrime->getWheelIndex();
    uint_t sievingPrime  = wPrime->getSievingPrime();
    unsetBit(sieve, sievingPrime, &multipleIndex, &wheelIndex);
    uint_t segment = getSegment(&multipleIndex);
    if (!bucketCache_[segment].store(sievingPrime, multipleIndex, wheelIndex))
      pushBucket(segment);
  }
}

} // namespace soe
