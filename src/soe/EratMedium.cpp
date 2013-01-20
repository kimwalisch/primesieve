///
/// @file   EratMedium.cpp
/// @brief  Segmented sieve of Eratosthenes optimized for medium
///         sieving primes.
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the New BSD License. See the
/// LICENSE file in the top level directory.
///

#include "config.h"
#include "EratMedium.h"
#include "WheelFactorization.h"
#include "primesieve_error.h"

#include <stdint.h>
#include <cassert>
#include <list>

namespace soe {

/// @param stop       Upper bound for sieving.
/// @param sieveSize  Sieve size in bytes.
/// @param limit      Sieving primes in EratMedium must be <= limit.
///
EratMedium::EratMedium(uint64_t stop, uint_t sieveSize, uint_t limit) :
  Modulo210Wheel_t(stop, sieveSize),
  limit_(limit)
{
  // ensure multipleIndex < 2^23 in crossOff()
  if (sieveSize > (1u << 22))
    throw primesieve_error("EratMedium: sieveSize must be <= 2^22, 4096 kilobytes");
  if (limit > sieveSize * 6)
    throw primesieve_error("EratMedium: limit must be <= sieveSize * 6");
  buckets_.push_back(Bucket());
}

/// Store a new sieving prime in EratMedium
void EratMedium::store(uint_t prime, uint_t multipleIndex, uint_t wheelIndex)
{
  assert(prime <= limit_);
  uint_t sievingPrime = prime / NUMBERS_PER_BYTE;
  if (!buckets_.back().store(sievingPrime, multipleIndex, wheelIndex))
    buckets_.push_back(Bucket());
}

/// Cross-off the multiples of medium sieving
/// primes from the sieve array.
///
void EratMedium::crossOff(byte_t* sieve, uint_t sieveSize)
{
  for (BucketIterator_t iter = buckets_.begin(); iter != buckets_.end(); ++iter)
    crossOff(sieve, sieveSize, *iter);
}

/// Cross-off the multiples of the sieving primes within the current
/// bucket. This is an implementation of the segmented sieve of
/// Eratosthenes with wheel factorization optimized for medium sieving
/// primes that have a few multiples per segment. This algorithm uses
/// a modulo 210 wheel that skips multiples of 2, 3, 5 and 7.
///
void EratMedium::crossOff(byte_t* sieve, uint_t sieveSize, Bucket& bucket)
{
  WheelPrime* wPrime = bucket.begin();
  WheelPrime* end    = bucket.end();

  // process 2 sieving primes per loop iteration to
  // increase instruction level parallelism
  for (; wPrime + 2 <= end; wPrime += 2) {
    uint_t multipleIndex0 = wPrime[0].getMultipleIndex();
    uint_t wheelIndex0    = wPrime[0].getWheelIndex();
    uint_t sievingPrime0  = wPrime[0].getSievingPrime();
    uint_t multipleIndex1 = wPrime[1].getMultipleIndex();
    uint_t wheelIndex1    = wPrime[1].getWheelIndex();
    uint_t sievingPrime1  = wPrime[1].getSievingPrime();
    // cross-off the multiples (unset bits) of sievingPrime
    // @see unsetBit() in WheelFactorization.h
    while (multipleIndex0 < sieveSize) {
      unsetBit(sieve, sievingPrime0, &multipleIndex0, &wheelIndex0);
      if (multipleIndex1 >= sieveSize) break;
      unsetBit(sieve, sievingPrime1, &multipleIndex1, &wheelIndex1);
    }
    while (multipleIndex0 < sieveSize) unsetBit(sieve, sievingPrime0, &multipleIndex0, &wheelIndex0);
    while (multipleIndex1 < sieveSize) unsetBit(sieve, sievingPrime1, &multipleIndex1, &wheelIndex1);
    multipleIndex0 -= sieveSize;
    multipleIndex1 -= sieveSize;
    wPrime[0].set(multipleIndex0, wheelIndex0);
    wPrime[1].set(multipleIndex1, wheelIndex1);
  }

  if (wPrime != end) {
    uint_t multipleIndex = wPrime->getMultipleIndex();
    uint_t wheelIndex    = wPrime->getWheelIndex();
    uint_t sievingPrime  = wPrime->getSievingPrime();
    while (multipleIndex < sieveSize)
      unsetBit(sieve, sievingPrime, &multipleIndex, &wheelIndex);
    multipleIndex -= sieveSize;
    wPrime->set(multipleIndex, wheelIndex);
  }
}

} // namespace soe
