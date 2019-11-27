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
/// Copyright (C) 2019 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/EratBig.hpp>
#include <primesieve/Bucket.hpp>
#include <primesieve/MemoryPool.hpp>
#include <primesieve/pmath.hpp>
#include <primesieve/primesieve_error.hpp>
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

  Wheel::init(stop, sieveSize);

  enabled_ = true;
  maxPrime_ = maxPrime;
  log2SieveSize_ = ilog2(sieveSize);
  moduloSieveSize_ = sieveSize - 1;

  uint64_t maxSievingPrime = maxPrime_ / 30;
  uint64_t maxNextMultiple = maxSievingPrime * getMaxFactor() + getMaxFactor();
  uint64_t maxMultipleIndex = sieveSize - 1 + maxNextMultiple;
  uint64_t maxSegmentCount = maxMultipleIndex >> log2SieveSize_;
  uint64_t size = maxSegmentCount + 1;

  sievingPrimes_.resize(size);
}

/// Add a new sieving prime
void EratBig::storeSievingPrime(uint64_t prime, uint64_t multipleIndex, uint64_t wheelIndex)
{
  assert(prime <= maxPrime_);
  uint64_t sievingPrime = prime / 30;
  uint64_t segment = multipleIndex >> log2SieveSize_;
  multipleIndex &= moduloSieveSize_;

  if (memoryPool_.isFullBucket(sievingPrimes_[segment]))
    memoryPool_.addBucket(sievingPrimes_[segment]);

  sievingPrimes_[segment]++->set(sievingPrime, multipleIndex, wheelIndex);
}

/// Iterate over the buckets related to the current segment
/// and for each bucket execute crossOff() to remove
/// the multiples of its sieving primes.
///
void EratBig::crossOff(uint8_t* sieve)
{
  while (sievingPrimes_[0])
  {
    Bucket* bucket = memoryPool_.getBucket(sievingPrimes_[0]);
    bucket->setEnd(sievingPrimes_[0]);
    sievingPrimes_[0] = nullptr;

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

/// Removes the next multiple of each sieving prime from the
/// sieve array. After the next multiple of a sieving prime
/// has been removed we calculate its next multiple and
/// determine in which segment that multiple will occur. Then
/// we move the sieving prime to the list related to the
/// previously computed segment.
///
void EratBig::crossOff(uint8_t* sieve, Bucket* bucket)
{
  SievingPrime* prime = bucket->begin();
  SievingPrime* end = bucket->end();
  SievingPrime** sievingPrimes = &sievingPrimes_[0];
  uint64_t moduloSieveSize = moduloSieveSize_;
  uint64_t log2SieveSize = log2SieveSize_;

  // Process 2 sieving primes per loop iteration to
  // increase instruction level parallelism.
  for (; prime <= end - 2; prime += 2)
  {
    uint64_t multipleIndex0 = prime[0].getMultipleIndex();
    uint64_t wheelIndex0    = prime[0].getWheelIndex();
    uint64_t sievingPrime0  = prime[0].getSievingPrime();
    uint64_t multipleIndex1 = prime[1].getMultipleIndex();
    uint64_t wheelIndex1    = prime[1].getWheelIndex();
    uint64_t sievingPrime1  = prime[1].getSievingPrime();

    // Cross-off the current multiple (unset bit)
    // and calculate the next multiple.
    unsetBit(sieve, sievingPrime0, &multipleIndex0, &wheelIndex0);
    uint64_t segment0 = multipleIndex0 >> log2SieveSize;
    multipleIndex0 &= moduloSieveSize;

    if (memoryPool_.isFullBucket(sievingPrimes[segment0]))
      memoryPool_.addBucket(sievingPrimes[segment0]);

    // The next multiple of the sieving prime will
    // occur in segment0. Hence we move the
    // sieving prime to the list which corresponds
    // to that segment.
    sievingPrimes[segment0]++->set(sievingPrime0, multipleIndex0, wheelIndex0);

    // Process the 2nd sieving prime
    unsetBit(sieve, sievingPrime1, &multipleIndex1, &wheelIndex1);
    uint64_t segment1 = multipleIndex1 >> log2SieveSize;
    multipleIndex1 &= moduloSieveSize;

    if (memoryPool_.isFullBucket(sievingPrimes[segment1]))
      memoryPool_.addBucket(sievingPrimes[segment1]);

    sievingPrimes[segment1]++->set(sievingPrime1, multipleIndex1, wheelIndex1);
  }

  if (prime != end)
  {
    uint64_t multipleIndex = prime->getMultipleIndex();
    uint64_t wheelIndex    = prime->getWheelIndex();
    uint64_t sievingPrime  = prime->getSievingPrime();

    unsetBit(sieve, sievingPrime, &multipleIndex, &wheelIndex);
    uint64_t segment = multipleIndex >> log2SieveSize;
    multipleIndex &= moduloSieveSize;

    if (memoryPool_.isFullBucket(sievingPrimes[segment]))
      memoryPool_.addBucket(sievingPrimes[segment]);

    sievingPrimes[segment]++->set(sievingPrime, multipleIndex, wheelIndex);
  }
}

} // namespace
