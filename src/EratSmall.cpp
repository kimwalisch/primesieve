///
/// @file   EratSmall.cpp
/// @brief  EratSmall is a segmented sieve of Eratosthenes
///         implementation optimized for small sieving primes. Since
///         each small sieving prime has many multiple occurrences per
///         segment the initialization overhead of the sieving primes
///         at the beginning of each segment is not really important
///         for performance. What matters is that crossing off
///         multiples uses as few instructions as possible since there
///         are so many multiples.
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include "EratSmall.hpp"
#include "Bucket.hpp"

#include <primesieve/bits.hpp>
#include <primesieve/macros.hpp>
#include <primesieve/pmath.hpp>

#include <stdint.h>
#include <algorithm>

namespace primesieve {

namespace {

template <uint8_t BASE,
          uint8_t MAX_OFFSET_ADD,
          uint8_t LOOP_ADD,
          uint8_t DIST_ADD_0, uint8_t DIST_ADD_1, uint8_t DIST_ADD_2, uint8_t DIST_ADD_3,
          uint8_t DIST_ADD_4, uint8_t DIST_ADD_5, uint8_t DIST_ADD_6, uint8_t DIST_ADD_7,
          uint8_t OFF_0, uint8_t OFF_1, uint8_t OFF_2, uint8_t OFF_3,
          uint8_t OFF_4, uint8_t OFF_5, uint8_t OFF_6, uint8_t OFF_7,
          uint8_t MASK_0, uint8_t MASK_1, uint8_t MASK_2, uint8_t MASK_3,
          uint8_t MASK_4, uint8_t MASK_5, uint8_t MASK_6, uint8_t MASK_7>
void crossOffByResidue(Vector<SievingPrime>& primes,
                       uint8_t* sieve,
                       std::size_t sieveSize)
{
  #define CHECK_FINISHED(wheelIndex) \
    if (i >= sieveSize) \
    { \
      std::size_t multipleIndex = i - sieveSize; \
      prime.set(multipleIndex, wheelIndex); \
      goto next_iteration; \
    }

  for (auto& prime : primes)
  {
    std::size_t sievingPrime = prime.getSievingPrime();
    std::size_t i = prime.getMultipleIndex();
    std::size_t wheelIndex = prime.getWheelIndex();
    std::size_t state = wheelIndex & 7;
    ASSERT(wheelIndex >= BASE);
    ASSERT(wheelIndex <= BASE + 7);

    std::size_t maxOffset = sievingPrime * 28 + MAX_OFFSET_ADD;
    std::size_t limit = std::max(sieveSize, maxOffset) - maxOffset;
    std::size_t loopDist = sievingPrime * 30 + LOOP_ADD;
    std::size_t adv0 = sievingPrime * 6 + DIST_ADD_0;
    std::size_t adv1 = sievingPrime * 4 + DIST_ADD_1;
    std::size_t adv2 = sievingPrime * 2 + DIST_ADD_2;
    std::size_t adv3 = sievingPrime * 4 + DIST_ADD_3;
    std::size_t adv4 = sievingPrime * 2 + DIST_ADD_4;
    std::size_t adv5 = sievingPrime * 4 + DIST_ADD_5;
    std::size_t adv6 = sievingPrime * 6 + DIST_ADD_6;
    std::size_t adv7 = sievingPrime * 2 + DIST_ADD_7;
    std::size_t s0 = sievingPrime *  0 + OFF_0;
    std::size_t s1 = sievingPrime *  6 + OFF_1;
    std::size_t s2 = sievingPrime * 10 + OFF_2;
    std::size_t s3 = sievingPrime * 12 + OFF_3;
    std::size_t s4 = sievingPrime * 16 + OFF_4;
    std::size_t s5 = sievingPrime * 18 + OFF_5;
    std::size_t s6 = sievingPrime * 22 + OFF_6;
    std::size_t s7 = sievingPrime * 28 + OFF_7;

    // Get ready for loop unrolling.
    for (; state; state = (state + 1) & 7)
    {
      wheelIndex = BASE + state;
      CHECK_FINISHED(wheelIndex);

      switch (state)
      {
        case 1: sieve[i] &= MASK_1; i += adv1; break;
        case 2: sieve[i] &= MASK_2; i += adv2; break;
        case 3: sieve[i] &= MASK_3; i += adv3; break;
        case 4: sieve[i] &= MASK_4; i += adv4; break;
        case 5: sieve[i] &= MASK_5; i += adv5; break;
        case 6: sieve[i] &= MASK_6; i += adv6; break;
        case 7: sieve[i] &= MASK_7; i += adv7; break;
        default: UNREACHABLE;
      }
    }

    // Each iteration removes the next 8
    // multiples of the sievingPrime.
    for (; i < limit; i += loopDist)
    {
      sieve[i + s0] &= MASK_0;
      sieve[i + s1] &= MASK_1;
      sieve[i + s2] &= MASK_2;
      sieve[i + s3] &= MASK_3;
      sieve[i + s4] &= MASK_4;
      sieve[i + s5] &= MASK_5;
      sieve[i + s6] &= MASK_6;
      sieve[i + s7] &= MASK_7;
    }

    // Cross off the last few multiples where i >= limit.
    for (;; state = (state + 1) & 7)
    {
      wheelIndex = BASE + state;
      CHECK_FINISHED(wheelIndex);

      switch (state)
      {
        case 0: sieve[i] &= MASK_0; i += adv0; break;
        case 1: sieve[i] &= MASK_1; i += adv1; break;
        case 2: sieve[i] &= MASK_2; i += adv2; break;
        case 3: sieve[i] &= MASK_3; i += adv3; break;
        case 4: sieve[i] &= MASK_4; i += adv4; break;
        case 5: sieve[i] &= MASK_5; i += adv5; break;
        case 6: sieve[i] &= MASK_6; i += adv6; break;
        case 7: sieve[i] &= MASK_7; i += adv7; break;
        default: UNREACHABLE;
      }
    }

    next_iteration:;
  }

  #undef CHECK_FINISHED
}

} // namespace

/// @stop:        Upper bound for sieving
/// @l1CacheSize: CPU L1 cache size
/// @maxPrime:    Sieving primes <= maxPrime
///
void EratSmall::init(uint64_t stop,
                     uint64_t l1CacheSize,
                     uint64_t maxPrime)
{
  ASSERT((maxPrime / 30) * getMaxFactor() + getMaxFactor() <= SievingPrime::MAX_MULTIPLEINDEX);
  static_assert(config::FACTOR_ERATSMALL <= 4.5,
               "config::FACTOR_ERATSMALL > 4.5 causes multipleIndex overflow 23-bits!");

  stop_ = stop;
  maxPrime_ = maxPrime;
  l1CacheSize_ = (std::size_t) l1CacheSize;
  std::size_t count = primeCountUpper(maxPrime);
  for (auto& primes : primeVectors_)
  {
    primes.clear();
    primes.reserve((count / 8) + 64);
  }
}

/// Add a new sieving prime to EratSmall
void EratSmall::storeSievingPrime(uint64_t prime,
                                  uint64_t multipleIndex,
                                  uint64_t wheelIndex)
{
  ASSERT(prime <= maxPrime_);
  uint64_t sievingPrime = prime / 30;
  uint64_t vectorIndex = wheelOffsets_[prime % 30] / 8;
  primeVectors_[vectorIndex].emplace_back(sievingPrime, multipleIndex, wheelIndex);
}

bool EratSmall::hasSievingPrimes() const
{
  for (const auto& primes : primeVectors_)
    if (!primes.empty())
      return true;

  return false;
}

/// Both EratMedium and EratBig usually run fastest using a
/// sieve size that is slightly smaller than the CPU's L2 cache
/// size. EratSmall however, runs fastest using a smaller sieve
/// size that matches the CPU's L1 cache size.
///
/// @sieveSize:   EratBig & EratMedium sieve size
/// @l1CacheSize: EratSmall sieve size
///
void EratSmall::crossOff(Vector<uint8_t>& sieve)
{
  for (std::size_t i = 0; i < sieve.size(); i += l1CacheSize_)
  {
    std::size_t sieveSize = std::min(l1CacheSize_, sieve.size() - i);
    crossOff(&sieve[i], sieveSize);
  }
}

/// Segmented sieve of Eratosthenes with wheel factorization
/// optimized for small sieving primes that have many multiples
/// per segment. This algorithm uses a hardcoded modulo 30
/// wheel that skips multiples of 2, 3 and 5.
///
void EratSmall::crossOff(uint8_t* sieve, std::size_t sieveSize)
{
  crossOff0(primeVectors_[0], sieve, sieveSize);
  crossOff1(primeVectors_[1], sieve, sieveSize);
  crossOff2(primeVectors_[2], sieve, sieveSize);
  crossOff3(primeVectors_[3], sieve, sieveSize);
  crossOff4(primeVectors_[4], sieve, sieveSize);
  crossOff5(primeVectors_[5], sieve, sieveSize);
  crossOff6(primeVectors_[6], sieve, sieveSize);
  crossOff7(primeVectors_[7], sieve, sieveSize);
}

void EratSmall::crossOff0(Vector<SievingPrime>& primes, uint8_t* sieve, std::size_t sieveSize)
{
  crossOffByResidue<0, 6, 7,
                    1, 1, 0, 1, 1, 1, 1, 1,
                    0, 1, 2, 2, 3, 4, 5, 6,
                    BIT0, BIT4, BIT3, BIT7, BIT6, BIT2, BIT1, BIT5>(primes, sieve, sieveSize);
}

void EratSmall::crossOff1(Vector<SievingPrime>& primes, uint8_t* sieve, std::size_t sieveSize)
{
  crossOffByResidue<8, 10, 11,
                    2, 1, 1, 2, 0, 2, 2, 1,
                    0, 2, 3, 4, 6, 6, 8, 10,
                    BIT1, BIT3, BIT7, BIT5, BIT0, BIT6, BIT2, BIT4>(primes, sieve, sieveSize);
}

void EratSmall::crossOff2(Vector<SievingPrime>& primes, uint8_t* sieve, std::size_t sieveSize)
{
  crossOffByResidue<16, 12, 13,
                    2, 2, 1, 2, 1, 1, 3, 1,
                    0, 2, 4, 5, 7, 8, 9, 12,
                    BIT2, BIT7, BIT5, BIT4, BIT1, BIT0, BIT6, BIT3>(primes, sieve, sieveSize);
}

void EratSmall::crossOff3(Vector<SievingPrime>& primes, uint8_t* sieve, std::size_t sieveSize)
{
  crossOffByResidue<24, 16, 17,
                    3, 3, 1, 2, 1, 2, 4, 1,
                    0, 3, 6, 7, 9, 10, 12, 16,
                    BIT3, BIT6, BIT0, BIT1, BIT4, BIT5, BIT7, BIT2>(primes, sieve, sieveSize);
}

void EratSmall::crossOff4(Vector<SievingPrime>& primes, uint8_t* sieve, std::size_t sieveSize)
{
  crossOffByResidue<32, 18, 19,
                    4, 2, 2, 2, 1, 3, 4, 1,
                    0, 4, 6, 8, 10, 11, 14, 18,
                    BIT4, BIT2, BIT6, BIT0, BIT5, BIT7, BIT3, BIT1>(primes, sieve, sieveSize);
}

void EratSmall::crossOff5(Vector<SievingPrime>& primes, uint8_t* sieve, std::size_t sieveSize)
{
  crossOffByResidue<40, 22, 23,
                    5, 3, 1, 3, 2, 3, 5, 1,
                    0, 5, 8, 9, 12, 14, 17, 22,
                    BIT5, BIT1, BIT2, BIT6, BIT7, BIT3, BIT4, BIT0>(primes, sieve, sieveSize);
}

void EratSmall::crossOff6(Vector<SievingPrime>& primes, uint8_t* sieve, std::size_t sieveSize)
{
  crossOffByResidue<48, 27, 29,
                    6, 4, 2, 4, 2, 4, 5, 2,
                    0, 6, 10, 12, 16, 18, 22, 27,
                    BIT6, BIT5, BIT4, BIT3, BIT2, BIT1, BIT0, BIT7>(primes, sieve, sieveSize);
}

void EratSmall::crossOff7(Vector<SievingPrime>& primes, uint8_t* sieve, std::size_t sieveSize)
{
  crossOffByResidue<56, 1, 1,
                    1, 0, 0, 0, 0, 0, 0, 0,
                    0, 1, 1, 1, 1, 1, 1, 1,
                    BIT7, BIT0, BIT1, BIT2, BIT3, BIT4, BIT5, BIT6>(primes, sieve, sieveSize);
}

} // namespace
