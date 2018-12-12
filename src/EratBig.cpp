///
/// @file   EratBig.cpp
/// @brief  Segmented sieve of Eratosthenes optimized for big sieving
///         primes. This is an optimized implementation of Tomas
///         Oliveira e Silva's cache-friendly bucket sieve algorithm:
///         http://www.ieeta.pt/~tos/software/prime_sieve.html
///         The idea is that for each segment we keep a list of buckets
///         which contain the sieving primes that have a multiple
///         occurrence in that segment. When we then cross off the
///         multiples from the current segment we avoid processing
///         sieving primes that do not have a multiple occurrence in
///         the current segment.
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/EratBig.hpp>
#include <primesieve/Bucket.hpp>
#include <primesieve/MemoryPool.hpp>
#include <primesieve/pmath.hpp>
#include <primesieve/primesieve_error.hpp>
#include <primesieve/types.hpp>
#include <primesieve/Wheel.hpp>

#include <stdint.h>
#include <cassert>
#include <algorithm>
#include <vector>

namespace primesieve {

/// @stop:      Upper bound for sieving
/// @sieveSize: Sieve size in bytes
/// @maxPrime:  Sieving primes <= maxPrime
///
void EratBig::init(uint64_t stop, uint64_t sieveSize, uint64_t maxPrime)
{
  // '>> log2SieveSize' requires power of 2 sieveSize
  if (!isPow2(sieveSize))
    throw primesieve_error("EratBig: sieveSize is not a power of 2");

  enabled_ = true;
  maxPrime_ = maxPrime;
  log2SieveSize_ = ilog2(sieveSize);
  moduloSieveSize_ = sieveSize - 1;

  Wheel::init(stop, sieveSize);
  init(sieveSize);
}

void EratBig::init(uint64_t sieveSize)
{
  uint64_t maxSievingPrime = maxPrime_ / 30;
  uint64_t maxNextMultiple = maxSievingPrime * getMaxFactor() + getMaxFactor();
  uint64_t maxMultipleIndex = sieveSize - 1 + maxNextMultiple;
  uint64_t maxSegmentCount = maxMultipleIndex >> log2SieveSize_;
  uint64_t size = maxSegmentCount + 1;

  sievingPrimes_.resize(size);

  for (SievingPrime*& sievingPrime : sievingPrimes_)
    memoryPool_.reset(sievingPrime);
}

/// Add a new sieving prime
void EratBig::storeSievingPrime(uint64_t prime, uint64_t multipleIndex, uint64_t wheelIndex)
{
  assert(prime <= maxPrime_);
  uint64_t sievingPrime = prime / 30;
  uint64_t segment = multipleIndex >> log2SieveSize_;
  multipleIndex &= moduloSieveSize_;

  sievingPrimes_[segment]++->set(sievingPrime, multipleIndex, wheelIndex);
  if (memoryPool_.isFullBucket(sievingPrimes_[segment]))
    memoryPool_.addBucket(sievingPrimes_[segment]);
}

/// Iterate over the buckets related to the current segment
/// and for each bucket execute crossOff() to remove
/// the multiples of its sieving primes.
///
void EratBig::crossOff(byte_t* sieve)
{
  while (true)
  {
    Bucket* bucket = memoryPool_.getBucket(sievingPrimes_[0]);
    bucket->setEnd(sievingPrimes_[0]);
    if (bucket->empty() && !bucket->hasNext())
      break;

    memoryPool_.reset(sievingPrimes_[0]);

    while (bucket)
    {
      crossOff(sieve, bucket);
      Bucket* processed = bucket;
      bucket = bucket->next();
      memoryPool_.freeBucket(processed);
    }
  }

  // Move the sieving primes related to the next segment to
  // the 1st position so that they will be used when
  // sieving the next segment.
  std::rotate(sievingPrimes_.begin(),
              sievingPrimes_.begin() + 1,
              sievingPrimes_.end());
}

/// Segmented sieve of Eratosthenes with wheel factorization
/// optimized for big sieving primes that have very few
/// multiples per segment. Cross-off the next multiple of
/// each sieving prime in the current bucket.
///
void EratBig::crossOff(byte_t* sieve, Bucket* bucket)
{
  SievingPrime* prime = bucket->begin();
  SievingPrime* end = bucket->end();
  SievingPrime** sievingPrimes = &sievingPrimes_[0];
  uint64_t moduloSieveSize = moduloSieveSize_;
  uint64_t log2SieveSize = log2SieveSize_;

  // process 2 sieving primes per loop iteration to
  // increase instruction level parallelism
  for (; prime <= end - 2; prime += 2)
  {
    uint64_t multipleIndex0 = prime[0].getMultipleIndex();
    uint64_t wheelIndex0    = prime[0].getWheelIndex();
    uint64_t sievingPrime0  = prime[0].getSievingPrime();
    uint64_t multipleIndex1 = prime[1].getMultipleIndex();
    uint64_t wheelIndex1    = prime[1].getWheelIndex();
    uint64_t sievingPrime1  = prime[1].getSievingPrime();

    // cross-off the current multiple (unset bit)
    // and calculate the next multiple
    unsetBit(sieve, sievingPrime0, &multipleIndex0, &wheelIndex0);
    unsetBit(sieve, sievingPrime1, &multipleIndex1, &wheelIndex1);
    uint64_t segment0 = multipleIndex0 >> log2SieveSize;
    uint64_t segment1 = multipleIndex1 >> log2SieveSize;
    multipleIndex0 &= moduloSieveSize;
    multipleIndex1 &= moduloSieveSize;

    // move the sieving prime to the list related
    // to the segment of its next multiple
    sievingPrimes[segment0]++->set(sievingPrime0, multipleIndex0, wheelIndex0);
    if (memoryPool_.isFullBucket(sievingPrimes[segment0]))
      memoryPool_.addBucket(sievingPrimes[segment0]);

    sievingPrimes[segment1]++->set(sievingPrime1, multipleIndex1, wheelIndex1);
    if (memoryPool_.isFullBucket(sievingPrimes[segment1]))
      memoryPool_.addBucket(sievingPrimes[segment1]);
  }

  if (prime != end)
  {
    uint64_t multipleIndex = prime->getMultipleIndex();
    uint64_t wheelIndex    = prime->getWheelIndex();
    uint64_t sievingPrime  = prime->getSievingPrime();

    unsetBit(sieve, sievingPrime, &multipleIndex, &wheelIndex);
    uint64_t segment = multipleIndex >> log2SieveSize;
    multipleIndex &= moduloSieveSize;

    sievingPrimes[segment]++->set(sievingPrime, multipleIndex, wheelIndex);
    if (memoryPool_.isFullBucket(sievingPrimes[segment]))
      memoryPool_.addBucket(sievingPrimes[segment]);
  }
}

} // namespace
