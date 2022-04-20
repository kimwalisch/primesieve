///
/// @file   EratBig.cpp
/// @brief  EratBig is a segmented sieve of Eratosthenes
///         implementation optimized for big sieving primes. EratBig
///         is a highly optimized implementation of Tomas Oliveira e
///         Silva's cache-friendly bucket sieve algorithm:
///         http://www.ieeta.pt/~tos/software/prime_sieve.html
///         The idea is that for each segment we keep a list of buckets
///         which contain the sieving primes that have a multiple
///         occurrence in that segment. When we then cross off the
///         multiples from the current segment we avoid processing
///         sieving primes that do not have a multiple occurrence in
///         the current segment.
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/EratBig.hpp>
#include <primesieve/Bucket.hpp>
#include <primesieve/MemoryPool.hpp>
#include <primesieve/pmath.hpp>

#include <stdint.h>
#include <cassert>
#include <algorithm>
#include <vector>

namespace primesieve {

/// @stop:      Upper bound for sieving
/// @sieveSize: Sieve size in bytes
/// @maxPrime:  Sieving primes <= maxPrime
///
void EratBig::init(uint64_t stop,
                   uint64_t sieveSize,
                   uint64_t maxPrime,
                   MemoryPool& memoryPool)
{
  // '>> log2SieveSize' requires power of 2 sieveSize
  assert(isPow2(sieveSize));
  assert(sieveSize <= SievingPrime::MAX_MULTIPLEINDEX + 1);

  enabled_ = true;
  stop_ = stop;
  maxPrime_ = maxPrime;
  log2SieveSize_ = ilog2(sieveSize);
  moduloSieveSize_ = sieveSize - 1;
  memoryPool_ = &memoryPool;

  uint64_t maxSievingPrime = maxPrime_ / 30;
  uint64_t maxNextMultiple = maxSievingPrime * getMaxFactor() + getMaxFactor();
  uint64_t maxMultipleIndex = sieveSize - 1 + maxNextMultiple;
  uint64_t maxSegmentCount = maxMultipleIndex >> log2SieveSize_;
  uint64_t size = maxSegmentCount + 1;

  buckets_.resize(size);
}

/// Add a new sieving prime
void EratBig::storeSievingPrime(uint64_t prime, uint64_t multipleIndex, uint64_t wheelIndex)
{
  uint64_t sievingPrime = prime / 30;
  uint64_t segment = multipleIndex >> log2SieveSize_;
  multipleIndex &= moduloSieveSize_;

  assert(prime <= maxPrime_);
  assert(segment < buckets_.size());

  if (Bucket::isFull(buckets_[segment]))
    memoryPool_->addBucket(buckets_[segment]);

  buckets_[segment]++->set(sievingPrime, multipleIndex, wheelIndex);
}

/// Iterate over the buckets related to the current segment
/// and for each bucket execute crossOff() to remove
/// the multiples of its sieving primes.
///
void EratBig::crossOff(uint8_t* sieve)
{
  while (buckets_[0])
  {
    Bucket* bucket = Bucket::get(buckets_[0]);
    bucket->setEnd(buckets_[0]);
    buckets_[0] = nullptr;

    while (bucket)
    {
      crossOff(sieve, bucket);
      Bucket* processed = bucket;
      bucket = bucket->next();
      memoryPool_->freeBucket(processed);
    }
  }

  // Move the bucket related to the next segment to
  // the 1st position so that it will be used when
  // sieving the next segment.
  std::copy(buckets_.begin() + 1, buckets_.end(), buckets_.begin());
  buckets_.back() = nullptr;
}

/// Removes the next multiple of each sieving prime from the
/// sieve array. After the next multiple of a sieving prime
/// has been removed we calculate its next multiple and
/// determine in which segment that multiple will occur. Then
/// we move the sieving prime to the bucket list related to
/// the previously computed segment.
///
void EratBig::crossOff(uint8_t* sieve, Bucket* bucket)
{
  SievingPrime* prime = bucket->begin();
  SievingPrime* end = bucket->end();
  auto buckets = buckets_.data();
  uint64_t moduloSieveSize = moduloSieveSize_;
  uint64_t log2SieveSize = log2SieveSize_;
  MemoryPool& memoryPool = *memoryPool_;

  // Process 2 sieving primes per loop iteration to
  // increase instruction level parallelism.
  for (; prime < end - 1; prime += 2)
  {
    uint64_t multipleIndex0 = prime[0].getMultipleIndex();
    uint64_t wheelIndex0    = prime[0].getWheelIndex();
    uint64_t sievingPrime0  = prime[0].getSievingPrime();
    uint64_t multipleIndex1 = prime[1].getMultipleIndex();
    uint64_t wheelIndex1    = prime[1].getWheelIndex();
    uint64_t sievingPrime1  = prime[1].getSievingPrime();

    sieve[multipleIndex0] &= wheel210[wheelIndex0].unsetBit;
    sieve[multipleIndex1] &= wheel210[wheelIndex1].unsetBit;

    multipleIndex0 += wheel210[wheelIndex0].nextMultipleFactor * sievingPrime0;
    multipleIndex0 += wheel210[wheelIndex0].correct;
    wheelIndex0 = wheel210[wheelIndex0].next;
    multipleIndex1 += wheel210[wheelIndex1].nextMultipleFactor * sievingPrime1;
    multipleIndex1 += wheel210[wheelIndex1].correct;
    wheelIndex1 = wheel210[wheelIndex1].next;

    uint64_t segment0 = multipleIndex0 >> log2SieveSize;
    uint64_t segment1 = multipleIndex1 >> log2SieveSize;
    multipleIndex0 &= moduloSieveSize;
    multipleIndex1 &= moduloSieveSize;

    if_unlikely(Bucket::isFull(buckets[segment0]))
      memoryPool.addBucket(buckets[segment0]);
    buckets[segment0]++->set(sievingPrime0, multipleIndex0, wheelIndex0);

    if_unlikely(Bucket::isFull(buckets[segment1]))
      memoryPool.addBucket(buckets[segment1]);
    buckets[segment1]++->set(sievingPrime1, multipleIndex1, wheelIndex1);
  }

  // Process the last sieving prime
  if_unlikely(prime < end)
  {
    assert(prime + 1 == end);
    uint64_t multipleIndex = prime->getMultipleIndex();
    uint64_t wheelIndex    = prime->getWheelIndex();
    uint64_t sievingPrime  = prime->getSievingPrime();

    sieve[multipleIndex] &= wheel210[wheelIndex].unsetBit;
    multipleIndex += wheel210[wheelIndex].nextMultipleFactor * sievingPrime;
    multipleIndex += wheel210[wheelIndex].correct;
    wheelIndex = wheel210[wheelIndex].next;
    uint64_t segment = multipleIndex >> log2SieveSize;
    multipleIndex &= moduloSieveSize;

    if_unlikely(Bucket::isFull(buckets[segment]))
      memoryPool.addBucket(buckets[segment]);
    buckets[segment]++->set(sievingPrime, multipleIndex, wheelIndex);
  }
}

} // namespace
