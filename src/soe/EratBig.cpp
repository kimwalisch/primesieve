///
/// @file   EratBig.cpp
/// @brief  Segmented sieve of Eratosthenes optimized for big sieving
///         primes. This is my implementation of Tomas Oliveira e
///         Silva's cache-friendly segmented sieve of Eratosthenes:
/// @see    http://www.ieeta.pt/~tos/software/prime_sieve.html
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the New BSD License. See the
/// LICENSE file in the top level directory.
///

#include "config.h"
#include "EratBig.h"
#include "WheelFactorization.h"
#include "SieveOfEratosthenes.h"
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
  setListsSize(sieveSize);
  init();
}

EratBig::~EratBig()
{
  for (PointerIterator_t iter = pointers_.begin(); iter != pointers_.end(); ++iter)
    delete[] *iter;
}

void EratBig::setListsSize(uint_t sieveSize)
{
  uint_t maxSievingPrime  = limit_ / SieveOfEratosthenes::NUMBERS_PER_BYTE;
  uint_t maxNextMultiple  = maxSievingPrime * getMaxFactor() + getMaxFactor();
  uint_t maxMultipleIndex = sieveSize - 1 + maxNextMultiple;
  uint_t maxSegmentCount  = maxMultipleIndex >> log2SieveSize_;
  uint_t size = maxSegmentCount + 1;
  lists_.resize(size, NULL);
}

void EratBig::init()
{
  for (uint_t i = 0; i < lists_.size(); i++)
    pushBucket(lists_[i]);
}

void EratBig::moveBucket(Bucket& src, Bucket*& dest)
{
  src.setNext(dest);
  dest = &src;
}

/// Add an empty bucket to the front of list
void EratBig::pushBucket(Bucket*& list)
{
  /// if the stock_ is empty new buckets are allocated first
  if (stock_ == NULL) {
    Bucket* more = new Bucket[BUCKETS_PER_ALLOC];
    stock_ = &more[0];
    pointers_.push_back(more);
    for(int i = 0; i < BUCKETS_PER_ALLOC - 1; i++)
      more[i].setNext(&more[i + 1]);
    more[BUCKETS_PER_ALLOC - 1].setNext(NULL);
  }
  Bucket* emptyBucket = stock_;
  stock_ = stock_->next();
  moveBucket(*emptyBucket, list);
}

/// Get the bucket list related to the segment of the next
/// multiple (multipleIndex) of sievingPrime.
///
Bucket*& EratBig::getList(uint_t* multipleIndex)
{
  uint_t segment = *multipleIndex >> log2SieveSize_;
  *multipleIndex &= moduloSieveSize_;
  return lists_[segment];
}

/// Store a new sieving prime in EratBig
void EratBig::store(uint_t prime, uint_t multipleIndex, uint_t wheelIndex)
{
  assert(prime <= limit_);
  uint_t sievingPrime = prime / SieveOfEratosthenes::NUMBERS_PER_BYTE;
  Bucket*& list = getList(&multipleIndex);
  if (!list->store(sievingPrime, multipleIndex, wheelIndex))
    pushBucket(list);
}

/// Cross-off the multiples of big sieving
/// primes from the sieve array.
///
void EratBig::crossOff(uint8_t* sieve)
{
  // lists_[0] contains the buckets with the sieving primes
  // that have multiples in the current segment
  Bucket*& list = lists_[0];

  while (list->hasNext() || !list->empty()) {
    Bucket* bucket = list;
    list = NULL;
    pushBucket(list);
    do {
      crossOff(sieve, *bucket);
      Bucket* processed = bucket;
      bucket = bucket->next();
      processed->reset();
      moveBucket(*processed, stock_);
    } while (bucket != NULL);
  }

  // lists_[0] has been processed, thus the list related to
  // the next segment lists_[1] moves to lists_[0] ...
  std::rotate(lists_.begin(), lists_.begin() + 1, lists_.end());
}

/// Cross-off the next multiple of each sieving prime within the
/// current bucket. This is an implementation of the segmented sieve
/// of Eratosthenes with wheel factorization optimized for big sieving
/// primes that have very few multiples per segment. This algorithm
/// uses a modulo 210 wheel that skips multiples of 2, 3, 5 and 7.
///
void EratBig::crossOff(uint8_t* sieve, Bucket& bucket)
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
    Bucket*& list0 = getList(&multipleIndex0);
    Bucket*& list1 = getList(&multipleIndex1);
    // move the 2 sieving primes to the list related
    // to their next multiple
    if (!list0->store(sievingPrime0, multipleIndex0, wheelIndex0))
      pushBucket(list0);
    if (!list1->store(sievingPrime1, multipleIndex1, wheelIndex1))
      pushBucket(list1);
  }

  if (wPrime != end) {
    uint_t multipleIndex = wPrime->getMultipleIndex();
    uint_t wheelIndex    = wPrime->getWheelIndex();
    uint_t sievingPrime  = wPrime->getSievingPrime();
    unsetBit(sieve, sievingPrime, &multipleIndex, &wheelIndex);
    Bucket*& list = getList(&multipleIndex);
    if (!list->store(sievingPrime, multipleIndex, wheelIndex))
      pushBucket(list);
  }
}

} // namespace soe
