///
/// @file   EratMedium.cpp
/// @brief  Segmented sieve of Eratosthenes optimized for
///         medium sieving primes.
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/Bucket.hpp>
#include <primesieve/EratMedium.hpp>
#include <primesieve/primesieve_error.hpp>
#include <primesieve/pmath.hpp>
#include <primesieve/types.hpp>
#include <primesieve/Wheel.hpp>

#include <stdint.h>
#include <cassert>
#include <vector>

namespace primesieve {

/// @stop:      Upper bound for sieving
/// @sieveSize: Sieve size in bytes
/// @maxPrime:  Sieving primes <= maxPrime
///
void EratMedium::init(uint64_t stop, uint64_t sieveSize, uint64_t maxPrime)
{
  uint64_t maxSieveSize = 4096 << 10;

  if (sieveSize > maxSieveSize)
    throw primesieve_error("EratMedium: sieveSize > 4096 KiB");
  if (maxPrime > sieveSize * 5)
    throw primesieve_error("EratMedium: maxPrime > sieveSize * 5");

  enabled_ = true;
  maxPrime_ = maxPrime;
  Wheel::init(stop, sieveSize);

  size_t size = primeCountApprox(maxPrime);
  primes_.reserve(size);
}

/// Add a new sieving prime to EratMedium
void EratMedium::storeSievingPrime(uint64_t prime, uint64_t multipleIndex, uint64_t wheelIndex)
{
  assert(prime <= maxPrime_);
  uint64_t sievingPrime = prime / 30;
  primes_.emplace_back(sievingPrime, multipleIndex, wheelIndex);
}

/// Segmented sieve of Eratosthenes with wheel factorization
/// optimized for medium sieving primes that have a
/// few multiples per segment
///
void EratMedium::crossOff(byte_t* sieve, uint64_t sieveSize)
{
  size_t i = 0;
  size_t size = primes_.size();
  auto primes = primes_.data();

  // process 3 sieving primes per loop iteration to
  // increase instruction level parallelism
  for (; i < size - size % 3; i += 3)
  {
    uint64_t multipleIndex0 = primes[i + 0].getMultipleIndex();
    uint64_t wheelIndex0    = primes[i + 0].getWheelIndex();
    uint64_t sievingPrime0  = primes[i + 0].getSievingPrime();
    uint64_t multipleIndex1 = primes[i + 1].getMultipleIndex();
    uint64_t wheelIndex1    = primes[i + 1].getWheelIndex();
    uint64_t sievingPrime1  = primes[i + 1].getSievingPrime();
    uint64_t multipleIndex2 = primes[i + 2].getMultipleIndex();
    uint64_t wheelIndex2    = primes[i + 2].getWheelIndex();
    uint64_t sievingPrime2  = primes[i + 2].getSievingPrime();

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

    primes[i + 0].set(multipleIndex0, wheelIndex0);
    primes[i + 1].set(multipleIndex1, wheelIndex1);
    primes[i + 2].set(multipleIndex2, wheelIndex2);
  }

  // process remaining sieving primes
  for (; i < size; i++)
  {
    uint64_t multipleIndex = primes[i].getMultipleIndex();
    uint64_t wheelIndex    = primes[i].getWheelIndex();
    uint64_t sievingPrime  = primes[i].getSievingPrime();

    while (multipleIndex < sieveSize)
      unsetBit(sieve, sievingPrime, &multipleIndex, &wheelIndex);

    multipleIndex -= sieveSize;
    primes[i].set(multipleIndex, wheelIndex);
  }
}

} // namespace
