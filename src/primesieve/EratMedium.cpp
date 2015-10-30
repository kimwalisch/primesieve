///
/// @file   EratMedium.cpp
/// @brief  Segmented sieve of Eratosthenes optimized for medium
///         sieving primes.
///
/// Copyright (C) 2015 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/config.hpp>
#include <primesieve/EratMedium.hpp>
#include <primesieve/WheelFactorization.hpp>
#include <primesieve/primesieve_error.hpp>

#include <stdint.h>
#include <cassert>
#include <list>

namespace primesieve {

/// @param stop       Upper bound for sieving.
/// @param sieveSize  Sieve size in bytes.
/// @param limit      Sieving primes in EratMedium must be <= limit.
///
EratMedium::EratMedium(uint64_t stop, uint_t sieveSize, uint_t limit) :
  Modulo210Wheel_t(stop, sieveSize),
  limit_(limit)
{
  // ensure multipleIndex < 2^23 in crossOff()
  if (sieveSize > (1u << 21))
    throw primesieve_error("EratMedium: sieveSize must be <= 2^21, 2048 kilobytes");
  if (limit > sieveSize * 9)
    throw primesieve_error("EratMedium: limit must be <= sieveSize * 9");
  buckets_.push_back(Bucket());
}

/// Add a new sieving prime to EratMedium
void EratMedium::storeSievingPrime(uint_t prime, uint_t multipleIndex, uint_t wheelIndex)
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
  SievingPrime* sPrime = bucket.begin();
  SievingPrime* sEnd = bucket.end();

  // process 3 sieving primes per loop iteration to
  // increase instruction level parallelism
  for (; sPrime + 3 <= sEnd; sPrime += 3)
  {
    uint_t multipleIndex0 = sPrime[0].getMultipleIndex();
    uint_t wheelIndex0    = sPrime[0].getWheelIndex();
    uint_t sievingPrime0  = sPrime[0].getSievingPrime();
    uint_t multipleIndex1 = sPrime[1].getMultipleIndex();
    uint_t wheelIndex1    = sPrime[1].getWheelIndex();
    uint_t sievingPrime1  = sPrime[1].getSievingPrime();
    uint_t multipleIndex2 = sPrime[2].getMultipleIndex();
    uint_t wheelIndex2    = sPrime[2].getWheelIndex();
    uint_t sievingPrime2  = sPrime[2].getSievingPrime();

    // cross-off the multiples (unset bits) of sievingPrime
    // @see unsetBit() in WheelFactorization.hpp
    while (multipleIndex0 < sieveSize)
    {
      unsetBit(sieve, sievingPrime0, &multipleIndex0, &wheelIndex0);
      if (multipleIndex1 >= sieveSize) break;
      unsetBit(sieve, sievingPrime1, &multipleIndex1, &wheelIndex1);
      if (multipleIndex2 >= sieveSize) break;
      unsetBit(sieve, sievingPrime2, &multipleIndex2, &wheelIndex2);
    }

    while (multipleIndex0 < sieveSize) unsetBit(sieve, sievingPrime0, &multipleIndex0, &wheelIndex0);
    while (multipleIndex1 < sieveSize) unsetBit(sieve, sievingPrime1, &multipleIndex1, &wheelIndex1);
    while (multipleIndex2 < sieveSize) unsetBit(sieve, sievingPrime2, &multipleIndex2, &wheelIndex2);

    multipleIndex0 -= sieveSize;
    multipleIndex1 -= sieveSize;
    multipleIndex2 -= sieveSize;

    sPrime[0].set(multipleIndex0, wheelIndex0);
    sPrime[1].set(multipleIndex1, wheelIndex1);
    sPrime[2].set(multipleIndex2, wheelIndex2);
  }

  // process remaining sieving primes
  for (; sPrime != sEnd; sPrime++)
  {
    uint_t multipleIndex = sPrime->getMultipleIndex();
    uint_t wheelIndex    = sPrime->getWheelIndex();
    uint_t sievingPrime  = sPrime->getSievingPrime();
    while (multipleIndex < sieveSize)
      unsetBit(sieve, sievingPrime, &multipleIndex, &wheelIndex);
    multipleIndex -= sieveSize;
    sPrime->set(multipleIndex, wheelIndex);
  }
}

} // namespace primesieve
