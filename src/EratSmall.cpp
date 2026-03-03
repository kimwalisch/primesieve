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
  primes_.reserve(count);
}

/// Add a new sieving prime to EratSmall
void EratSmall::storeSievingPrime(uint64_t prime,
                                  uint64_t multipleIndex,
                                  uint64_t wheelIndex)
{
  ASSERT(prime <= maxPrime_);
  uint64_t sievingPrime = prime / 30;
  primes_.emplace_back(sievingPrime, multipleIndex, wheelIndex);
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
  #define CHECK_FINISHED(wheelIndex) \
    if (i >= sieveSize) \
    { \
      std::size_t multipleIndex = i - sieveSize; \
      prime.set(multipleIndex, wheelIndex); \
      goto next_iteration; \
    }

  for (auto& prime : primes_)
  {
    std::size_t sievingPrime = prime.getSievingPrime();
    std::size_t i = prime.getMultipleIndex();
    std::size_t wheelIndex = prime.getWheelIndex();
    ASSERT(wheelIndex <= 63);

    switch (wheelIndex)
    {
      // sievingPrime % 30 == 7
      for (;;)
      {
        case 0: {
                  std::size_t maxOffset = sievingPrime * 28 + 6;
                  std::size_t limit = std::max(sieveSize, maxOffset) - maxOffset;

                  // Each iteration removes the next 8
                  // multiples of the sievingPrime.
                  for (; i < limit; i += sievingPrime * 30 + 7)
                  {
                    sieve[i + sievingPrime *  0 + 0] &= BIT0;
                    sieve[i + sievingPrime *  6 + 1] &= BIT4;
                    sieve[i + sievingPrime * 10 + 2] &= BIT3;
                    sieve[i + sievingPrime * 12 + 2] &= BIT7;
                    sieve[i + sievingPrime * 16 + 3] &= BIT6;
                    sieve[i + sievingPrime * 18 + 4] &= BIT2;
                    sieve[i + sievingPrime * 22 + 5] &= BIT1;
                    sieve[i + sievingPrime * 28 + 6] &= BIT5;
                  }
                }
                CHECK_FINISHED(0); sieve[i] &= BIT0; i += sievingPrime * 6 + 1; FALLTHROUGH;
        case 1: CHECK_FINISHED(1); sieve[i] &= BIT4; i += sievingPrime * 4 + 1; FALLTHROUGH;
        case 2: CHECK_FINISHED(2); sieve[i] &= BIT3; i += sievingPrime * 2 + 0; FALLTHROUGH;
        case 3: CHECK_FINISHED(3); sieve[i] &= BIT7; i += sievingPrime * 4 + 1; FALLTHROUGH;
        case 4: CHECK_FINISHED(4); sieve[i] &= BIT6; i += sievingPrime * 2 + 1; FALLTHROUGH;
        case 5: CHECK_FINISHED(5); sieve[i] &= BIT2; i += sievingPrime * 4 + 1; FALLTHROUGH;
        case 6: CHECK_FINISHED(6); sieve[i] &= BIT1; i += sievingPrime * 6 + 1; FALLTHROUGH;
        case 7: CHECK_FINISHED(7); sieve[i] &= BIT5; i += sievingPrime * 2 + 1;
      }

      // sievingPrime % 30 == 11
      for (;;)
      {
        case  8: {
                   std::size_t maxOffset = sievingPrime * 28 + 10;
                   std::size_t limit = std::max(sieveSize, maxOffset) - maxOffset;

                   for (; i < limit; i += sievingPrime * 30 + 11)
                   {
                     sieve[i + sievingPrime *  0 +  0] &= BIT1;
                     sieve[i + sievingPrime *  6 +  2] &= BIT3;
                     sieve[i + sievingPrime * 10 +  3] &= BIT7;
                     sieve[i + sievingPrime * 12 +  4] &= BIT5;
                     sieve[i + sievingPrime * 16 +  6] &= BIT0;
                     sieve[i + sievingPrime * 18 +  6] &= BIT6;
                     sieve[i + sievingPrime * 22 +  8] &= BIT2;
                     sieve[i + sievingPrime * 28 + 10] &= BIT4;
                   }
                 }
                 CHECK_FINISHED( 8); sieve[i] &= BIT1; i += sievingPrime * 6 + 2; FALLTHROUGH;
        case  9: CHECK_FINISHED( 9); sieve[i] &= BIT3; i += sievingPrime * 4 + 1; FALLTHROUGH;
        case 10: CHECK_FINISHED(10); sieve[i] &= BIT7; i += sievingPrime * 2 + 1; FALLTHROUGH;
        case 11: CHECK_FINISHED(11); sieve[i] &= BIT5; i += sievingPrime * 4 + 2; FALLTHROUGH;
        case 12: CHECK_FINISHED(12); sieve[i] &= BIT0; i += sievingPrime * 2 + 0; FALLTHROUGH;
        case 13: CHECK_FINISHED(13); sieve[i] &= BIT6; i += sievingPrime * 4 + 2; FALLTHROUGH;
        case 14: CHECK_FINISHED(14); sieve[i] &= BIT2; i += sievingPrime * 6 + 2; FALLTHROUGH;
        case 15: CHECK_FINISHED(15); sieve[i] &= BIT4; i += sievingPrime * 2 + 1;
      }

      // sievingPrime % 30 == 13
      for (;;)
      {
        case 16: {
                   std::size_t maxOffset = sievingPrime * 28 + 12;
                   std::size_t limit = std::max(sieveSize, maxOffset) - maxOffset;

                   for (; i < limit; i += sievingPrime * 30 + 13)
                   {
                     sieve[i + sievingPrime *  0 +  0] &= BIT2;
                     sieve[i + sievingPrime *  6 +  2] &= BIT7;
                     sieve[i + sievingPrime * 10 +  4] &= BIT5;
                     sieve[i + sievingPrime * 12 +  5] &= BIT4;
                     sieve[i + sievingPrime * 16 +  7] &= BIT1;
                     sieve[i + sievingPrime * 18 +  8] &= BIT0;
                     sieve[i + sievingPrime * 22 +  9] &= BIT6;
                     sieve[i + sievingPrime * 28 + 12] &= BIT3;
                   }
                 }
                 CHECK_FINISHED(16); sieve[i] &= BIT2; i += sievingPrime * 6 + 2; FALLTHROUGH;
        case 17: CHECK_FINISHED(17); sieve[i] &= BIT7; i += sievingPrime * 4 + 2; FALLTHROUGH;
        case 18: CHECK_FINISHED(18); sieve[i] &= BIT5; i += sievingPrime * 2 + 1; FALLTHROUGH;
        case 19: CHECK_FINISHED(19); sieve[i] &= BIT4; i += sievingPrime * 4 + 2; FALLTHROUGH;
        case 20: CHECK_FINISHED(20); sieve[i] &= BIT1; i += sievingPrime * 2 + 1; FALLTHROUGH;
        case 21: CHECK_FINISHED(21); sieve[i] &= BIT0; i += sievingPrime * 4 + 1; FALLTHROUGH;
        case 22: CHECK_FINISHED(22); sieve[i] &= BIT6; i += sievingPrime * 6 + 3; FALLTHROUGH;
        case 23: CHECK_FINISHED(23); sieve[i] &= BIT3; i += sievingPrime * 2 + 1;
      }

      // sievingPrime % 30 == 17
      for (;;)
      {
        case 24: {
                   std::size_t maxOffset = sievingPrime * 28 + 16;
                   std::size_t limit = std::max(sieveSize, maxOffset) - maxOffset;

                   for (; i < limit; i += sievingPrime * 30 + 17)
                   {
                     sieve[i + sievingPrime *  0 +  0] &= BIT3;
                     sieve[i + sievingPrime *  6 +  3] &= BIT6;
                     sieve[i + sievingPrime * 10 +  6] &= BIT0;
                     sieve[i + sievingPrime * 12 +  7] &= BIT1;
                     sieve[i + sievingPrime * 16 +  9] &= BIT4;
                     sieve[i + sievingPrime * 18 + 10] &= BIT5;
                     sieve[i + sievingPrime * 22 + 12] &= BIT7;
                     sieve[i + sievingPrime * 28 + 16] &= BIT2;
                   }
                 }
                 CHECK_FINISHED(24); sieve[i] &= BIT3; i += sievingPrime * 6 + 3; FALLTHROUGH;
        case 25: CHECK_FINISHED(25); sieve[i] &= BIT6; i += sievingPrime * 4 + 3; FALLTHROUGH;
        case 26: CHECK_FINISHED(26); sieve[i] &= BIT0; i += sievingPrime * 2 + 1; FALLTHROUGH;
        case 27: CHECK_FINISHED(27); sieve[i] &= BIT1; i += sievingPrime * 4 + 2; FALLTHROUGH;
        case 28: CHECK_FINISHED(28); sieve[i] &= BIT4; i += sievingPrime * 2 + 1; FALLTHROUGH;
        case 29: CHECK_FINISHED(29); sieve[i] &= BIT5; i += sievingPrime * 4 + 2; FALLTHROUGH;
        case 30: CHECK_FINISHED(30); sieve[i] &= BIT7; i += sievingPrime * 6 + 4; FALLTHROUGH;
        case 31: CHECK_FINISHED(31); sieve[i] &= BIT2; i += sievingPrime * 2 + 1;
      }

      // sievingPrime % 30 == 19
      for (;;)
      {
        case 32: {
                   std::size_t maxOffset = sievingPrime * 28 + 18;
                   std::size_t limit = std::max(sieveSize, maxOffset) - maxOffset;

                   for (; i < limit; i += sievingPrime * 30 + 19)
                   {
                     sieve[i + sievingPrime *  0 +  0] &= BIT4;
                     sieve[i + sievingPrime *  6 +  4] &= BIT2;
                     sieve[i + sievingPrime * 10 +  6] &= BIT6;
                     sieve[i + sievingPrime * 12 +  8] &= BIT0;
                     sieve[i + sievingPrime * 16 + 10] &= BIT5;
                     sieve[i + sievingPrime * 18 + 11] &= BIT7;
                     sieve[i + sievingPrime * 22 + 14] &= BIT3;
                     sieve[i + sievingPrime * 28 + 18] &= BIT1;
                   }
                 }
                 CHECK_FINISHED(32); sieve[i] &= BIT4; i += sievingPrime * 6 + 4; FALLTHROUGH;
        case 33: CHECK_FINISHED(33); sieve[i] &= BIT2; i += sievingPrime * 4 + 2; FALLTHROUGH;
        case 34: CHECK_FINISHED(34); sieve[i] &= BIT6; i += sievingPrime * 2 + 2; FALLTHROUGH;
        case 35: CHECK_FINISHED(35); sieve[i] &= BIT0; i += sievingPrime * 4 + 2; FALLTHROUGH;
        case 36: CHECK_FINISHED(36); sieve[i] &= BIT5; i += sievingPrime * 2 + 1; FALLTHROUGH;
        case 37: CHECK_FINISHED(37); sieve[i] &= BIT7; i += sievingPrime * 4 + 3; FALLTHROUGH;
        case 38: CHECK_FINISHED(38); sieve[i] &= BIT3; i += sievingPrime * 6 + 4; FALLTHROUGH;
        case 39: CHECK_FINISHED(39); sieve[i] &= BIT1; i += sievingPrime * 2 + 1;
      }

      // sievingPrime % 30 == 23
      for (;;)
      {
        case 40: {
                   std::size_t maxOffset = sievingPrime * 28 + 22;
                   std::size_t limit = std::max(sieveSize, maxOffset) - maxOffset;

                   for (; i < limit; i += sievingPrime * 30 + 23)
                   {
                     sieve[i + sievingPrime *  0 +  0] &= BIT5;
                     sieve[i + sievingPrime *  6 +  5] &= BIT1;
                     sieve[i + sievingPrime * 10 +  8] &= BIT2;
                     sieve[i + sievingPrime * 12 +  9] &= BIT6;
                     sieve[i + sievingPrime * 16 + 12] &= BIT7;
                     sieve[i + sievingPrime * 18 + 14] &= BIT3;
                     sieve[i + sievingPrime * 22 + 17] &= BIT4;
                     sieve[i + sievingPrime * 28 + 22] &= BIT0;
                   }
                 }
                 CHECK_FINISHED(40); sieve[i] &= BIT5; i += sievingPrime * 6 + 5; FALLTHROUGH;
        case 41: CHECK_FINISHED(41); sieve[i] &= BIT1; i += sievingPrime * 4 + 3; FALLTHROUGH;
        case 42: CHECK_FINISHED(42); sieve[i] &= BIT2; i += sievingPrime * 2 + 1; FALLTHROUGH;
        case 43: CHECK_FINISHED(43); sieve[i] &= BIT6; i += sievingPrime * 4 + 3; FALLTHROUGH;
        case 44: CHECK_FINISHED(44); sieve[i] &= BIT7; i += sievingPrime * 2 + 2; FALLTHROUGH;
        case 45: CHECK_FINISHED(45); sieve[i] &= BIT3; i += sievingPrime * 4 + 3; FALLTHROUGH;
        case 46: CHECK_FINISHED(46); sieve[i] &= BIT4; i += sievingPrime * 6 + 5; FALLTHROUGH;
        case 47: CHECK_FINISHED(47); sieve[i] &= BIT0; i += sievingPrime * 2 + 1;
      }

      // sievingPrime % 30 == 29
      for (;;)
      {
        case 48: {
                   std::size_t maxOffset = sievingPrime * 28 + 27;
                   std::size_t limit = std::max(sieveSize, maxOffset) - maxOffset;

                   for (; i < limit; i += sievingPrime * 30 + 29)
                   {
                     sieve[i + sievingPrime *  0 +  0] &= BIT6;
                     sieve[i + sievingPrime *  6 +  6] &= BIT5;
                     sieve[i + sievingPrime * 10 + 10] &= BIT4;
                     sieve[i + sievingPrime * 12 + 12] &= BIT3;
                     sieve[i + sievingPrime * 16 + 16] &= BIT2;
                     sieve[i + sievingPrime * 18 + 18] &= BIT1;
                     sieve[i + sievingPrime * 22 + 22] &= BIT0;
                     sieve[i + sievingPrime * 28 + 27] &= BIT7;
                   }
                 }
                 CHECK_FINISHED(48); sieve[i] &= BIT6; i += sievingPrime * 6 + 6; FALLTHROUGH;
        case 49: CHECK_FINISHED(49); sieve[i] &= BIT5; i += sievingPrime * 4 + 4; FALLTHROUGH;
        case 50: CHECK_FINISHED(50); sieve[i] &= BIT4; i += sievingPrime * 2 + 2; FALLTHROUGH;
        case 51: CHECK_FINISHED(51); sieve[i] &= BIT3; i += sievingPrime * 4 + 4; FALLTHROUGH;
        case 52: CHECK_FINISHED(52); sieve[i] &= BIT2; i += sievingPrime * 2 + 2; FALLTHROUGH;
        case 53: CHECK_FINISHED(53); sieve[i] &= BIT1; i += sievingPrime * 4 + 4; FALLTHROUGH;
        case 54: CHECK_FINISHED(54); sieve[i] &= BIT0; i += sievingPrime * 6 + 5; FALLTHROUGH;
        case 55: CHECK_FINISHED(55); sieve[i] &= BIT7; i += sievingPrime * 2 + 2;
      }

      // sievingPrime % 30 == 1
      for (;;)
      {
        case 56: {
                   std::size_t maxOffset = sievingPrime * 28 + 1;
                   std::size_t limit = std::max(sieveSize, maxOffset) - maxOffset;

                   for (; i < limit; i += sievingPrime * 30 + 1)
                   {
                     sieve[i + sievingPrime *  0 + 0] &= BIT7;
                     sieve[i + sievingPrime *  6 + 1] &= BIT0;
                     sieve[i + sievingPrime * 10 + 1] &= BIT1;
                     sieve[i + sievingPrime * 12 + 1] &= BIT2;
                     sieve[i + sievingPrime * 16 + 1] &= BIT3;
                     sieve[i + sievingPrime * 18 + 1] &= BIT4;
                     sieve[i + sievingPrime * 22 + 1] &= BIT5;
                     sieve[i + sievingPrime * 28 + 1] &= BIT6;
                   }
                 }
                 CHECK_FINISHED(56); sieve[i] &= BIT7; i += sievingPrime * 6 + 1; FALLTHROUGH;
        case 57: CHECK_FINISHED(57); sieve[i] &= BIT0; i += sievingPrime * 4 + 0; FALLTHROUGH;
        case 58: CHECK_FINISHED(58); sieve[i] &= BIT1; i += sievingPrime * 2 + 0; FALLTHROUGH;
        case 59: CHECK_FINISHED(59); sieve[i] &= BIT2; i += sievingPrime * 4 + 0; FALLTHROUGH;
        case 60: CHECK_FINISHED(60); sieve[i] &= BIT3; i += sievingPrime * 2 + 0; FALLTHROUGH;
        case 61: CHECK_FINISHED(61); sieve[i] &= BIT4; i += sievingPrime * 4 + 0; FALLTHROUGH;
        case 62: CHECK_FINISHED(62); sieve[i] &= BIT5; i += sievingPrime * 6 + 0; FALLTHROUGH;
        case 63: CHECK_FINISHED(63); sieve[i] &= BIT6; i += sievingPrime * 2 + 0;
      }

      default: UNREACHABLE;
    }

    next_iteration:;
  }
}

} // namespace
