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
/// Copyright (C) 2026 Kim Walisch, <kim.walisch@gmail.com>
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
void EratSmall::crossOff(Vector<uint64_t>& sieve)
{
  uint8_t* sieve8 = (uint8_t*) sieve.data();
  std::size_t sieveBytes = sieve.size() * sizeof(uint64_t);

  for (std::size_t i = 0; i < sieveBytes; i += l1CacheSize_)
  {
    std::size_t chunkSize = std::min(l1CacheSize_, sieveBytes - i);
    crossOff(&sieve8[i], chunkSize);
  }
}

/// Segmented sieve of Eratosthenes with wheel factorization
/// optimized for small sieving primes that have many multiples
/// per segment. This algorithm uses a hardcoded modulo 30
/// wheel that skips multiples of 2, 3 and 5.
///
void EratSmall::crossOff(uint8_t* sieve, std::size_t sieveBytes)
{
  #define CHECK_FINISHED(wheelIndex) \
    if (i0 >= sieveBytes) \
    { \
      std::size_t multipleIndex = i0 - sieveBytes; \
      prime.set(multipleIndex, wheelIndex); \
      goto next_iteration; \
    }

  for (auto& prime : primes_)
  {
    std::size_t sievingPrime = prime.getSievingPrime();
    std::size_t i0 = prime.getMultipleIndex();
    std::size_t wheelIndex = prime.getWheelIndex();
    ASSERT(wheelIndex <= 63);

    switch (wheelIndex)
    {
      // sievingPrime % 30 == 7
      for (;;)
      {
        case 0: {
                  std::size_t i1 = i0 + sievingPrime *  6 + 1;
                  std::size_t i2 = i0 + sievingPrime * 10 + 2;
                  std::size_t i3 = i0 + sievingPrime * 12 + 2;
                  std::size_t i4 = i0 + sievingPrime * 16 + 3;
                  std::size_t i5 = i0 + sievingPrime * 18 + 4;
                  std::size_t i6 = i0 + sievingPrime * 22 + 5;
                  std::size_t i7 = i0 + sievingPrime * 28 + 6;
                  std::size_t dist = sievingPrime * 30 + 7;

                  // Each iteration removes the next 8
                  // multiples of the sievingPrime.
                  while (i7 < sieveBytes)
                  {
                    sieve[i0] &= BIT0; i0 += dist;
                    sieve[i1] &= BIT4; i1 += dist;
                    sieve[i2] &= BIT3; i2 += dist;
                    sieve[i3] &= BIT7; i3 += dist;
                    sieve[i4] &= BIT6; i4 += dist;
                    sieve[i5] &= BIT2; i5 += dist;
                    sieve[i6] &= BIT1; i6 += dist;
                    sieve[i7] &= BIT5; i7 += dist;
                  }
                }
                CHECK_FINISHED(0); sieve[i0] &= BIT0; i0 += sievingPrime * 6 + 1; FALLTHROUGH;
        case 1: CHECK_FINISHED(1); sieve[i0] &= BIT4; i0 += sievingPrime * 4 + 1; FALLTHROUGH;
        case 2: CHECK_FINISHED(2); sieve[i0] &= BIT3; i0 += sievingPrime * 2 + 0; FALLTHROUGH;
        case 3: CHECK_FINISHED(3); sieve[i0] &= BIT7; i0 += sievingPrime * 4 + 1; FALLTHROUGH;
        case 4: CHECK_FINISHED(4); sieve[i0] &= BIT6; i0 += sievingPrime * 2 + 1; FALLTHROUGH;
        case 5: CHECK_FINISHED(5); sieve[i0] &= BIT2; i0 += sievingPrime * 4 + 1; FALLTHROUGH;
        case 6: CHECK_FINISHED(6); sieve[i0] &= BIT1; i0 += sievingPrime * 6 + 1; FALLTHROUGH;
        case 7: CHECK_FINISHED(7); sieve[i0] &= BIT5; i0 += sievingPrime * 2 + 1;
      }

      // sievingPrime % 30 == 11
      for (;;)
      {
        case  8: {
                   std::size_t i1 = i0 + sievingPrime *  6 +  2;
                   std::size_t i2 = i0 + sievingPrime * 10 +  3;
                   std::size_t i3 = i0 + sievingPrime * 12 +  4;
                   std::size_t i4 = i0 + sievingPrime * 16 +  6;
                   std::size_t i5 = i0 + sievingPrime * 18 +  6;
                   std::size_t i6 = i0 + sievingPrime * 22 +  8;
                   std::size_t i7 = i0 + sievingPrime * 28 + 10;
                   std::size_t dist = sievingPrime * 30 + 11;

                   // Each iteration removes the next 8
                   // multiples of the sievingPrime.
                   while (i7 < sieveBytes)
                   {
                     sieve[i0] &= BIT1; i0 += dist;
                     sieve[i1] &= BIT3; i1 += dist;
                     sieve[i2] &= BIT7; i2 += dist;
                     sieve[i3] &= BIT5; i3 += dist;
                     sieve[i4] &= BIT0; i4 += dist;
                     sieve[i5] &= BIT6; i5 += dist;
                     sieve[i6] &= BIT2; i6 += dist;
                     sieve[i7] &= BIT4; i7 += dist;
                   }
                 }
                 CHECK_FINISHED( 8); sieve[i0] &= BIT1; i0 += sievingPrime * 6 + 2; FALLTHROUGH;
        case  9: CHECK_FINISHED( 9); sieve[i0] &= BIT3; i0 += sievingPrime * 4 + 1; FALLTHROUGH;
        case 10: CHECK_FINISHED(10); sieve[i0] &= BIT7; i0 += sievingPrime * 2 + 1; FALLTHROUGH;
        case 11: CHECK_FINISHED(11); sieve[i0] &= BIT5; i0 += sievingPrime * 4 + 2; FALLTHROUGH;
        case 12: CHECK_FINISHED(12); sieve[i0] &= BIT0; i0 += sievingPrime * 2 + 0; FALLTHROUGH;
        case 13: CHECK_FINISHED(13); sieve[i0] &= BIT6; i0 += sievingPrime * 4 + 2; FALLTHROUGH;
        case 14: CHECK_FINISHED(14); sieve[i0] &= BIT2; i0 += sievingPrime * 6 + 2; FALLTHROUGH;
        case 15: CHECK_FINISHED(15); sieve[i0] &= BIT4; i0 += sievingPrime * 2 + 1;
      }

      // sievingPrime % 30 == 13
      for (;;)
      {
        case 16: {
                   std::size_t i1 = i0 + sievingPrime *  6 +  2;
                   std::size_t i2 = i0 + sievingPrime * 10 +  4;
                   std::size_t i3 = i0 + sievingPrime * 12 +  5;
                   std::size_t i4 = i0 + sievingPrime * 16 +  7;
                   std::size_t i5 = i0 + sievingPrime * 18 +  8;
                   std::size_t i6 = i0 + sievingPrime * 22 +  9;
                   std::size_t i7 = i0 + sievingPrime * 28 + 12;
                   std::size_t dist = sievingPrime * 30 + 13;

                   // Each iteration removes the next 8
                   // multiples of the sievingPrime.
                   while (i7 < sieveBytes)
                   {
                     sieve[i0] &= BIT2; i0 += dist;
                     sieve[i1] &= BIT7; i1 += dist;
                     sieve[i2] &= BIT5; i2 += dist;
                     sieve[i3] &= BIT4; i3 += dist;
                     sieve[i4] &= BIT1; i4 += dist;
                     sieve[i5] &= BIT0; i5 += dist;
                     sieve[i6] &= BIT6; i6 += dist;
                     sieve[i7] &= BIT3; i7 += dist;
                   }
                 }
                 CHECK_FINISHED(16); sieve[i0] &= BIT2; i0 += sievingPrime * 6 + 2; FALLTHROUGH;
        case 17: CHECK_FINISHED(17); sieve[i0] &= BIT7; i0 += sievingPrime * 4 + 2; FALLTHROUGH;
        case 18: CHECK_FINISHED(18); sieve[i0] &= BIT5; i0 += sievingPrime * 2 + 1; FALLTHROUGH;
        case 19: CHECK_FINISHED(19); sieve[i0] &= BIT4; i0 += sievingPrime * 4 + 2; FALLTHROUGH;
        case 20: CHECK_FINISHED(20); sieve[i0] &= BIT1; i0 += sievingPrime * 2 + 1; FALLTHROUGH;
        case 21: CHECK_FINISHED(21); sieve[i0] &= BIT0; i0 += sievingPrime * 4 + 1; FALLTHROUGH;
        case 22: CHECK_FINISHED(22); sieve[i0] &= BIT6; i0 += sievingPrime * 6 + 3; FALLTHROUGH;
        case 23: CHECK_FINISHED(23); sieve[i0] &= BIT3; i0 += sievingPrime * 2 + 1;
      }

      // sievingPrime % 30 == 17
      for (;;)
      {
        case 24: {
                   std::size_t i1 = i0 + sievingPrime *  6 +  3;
                   std::size_t i2 = i0 + sievingPrime * 10 +  6;
                   std::size_t i3 = i0 + sievingPrime * 12 +  7;
                   std::size_t i4 = i0 + sievingPrime * 16 +  9;
                   std::size_t i5 = i0 + sievingPrime * 18 + 10;
                   std::size_t i6 = i0 + sievingPrime * 22 + 12;
                   std::size_t i7 = i0 + sievingPrime * 28 + 16;
                   std::size_t dist = sievingPrime * 30 + 17;

                   // Each iteration removes the next 8
                   // multiples of the sievingPrime.
                   while (i7 < sieveBytes)
                   {
                     sieve[i0] &= BIT3; i0 += dist;
                     sieve[i1] &= BIT6; i1 += dist;
                     sieve[i2] &= BIT0; i2 += dist;
                     sieve[i3] &= BIT1; i3 += dist;
                     sieve[i4] &= BIT4; i4 += dist;
                     sieve[i5] &= BIT5; i5 += dist;
                     sieve[i6] &= BIT7; i6 += dist;
                     sieve[i7] &= BIT2; i7 += dist;
                   }
                 }
                 CHECK_FINISHED(24); sieve[i0] &= BIT3; i0 += sievingPrime * 6 + 3; FALLTHROUGH;
        case 25: CHECK_FINISHED(25); sieve[i0] &= BIT6; i0 += sievingPrime * 4 + 3; FALLTHROUGH;
        case 26: CHECK_FINISHED(26); sieve[i0] &= BIT0; i0 += sievingPrime * 2 + 1; FALLTHROUGH;
        case 27: CHECK_FINISHED(27); sieve[i0] &= BIT1; i0 += sievingPrime * 4 + 2; FALLTHROUGH;
        case 28: CHECK_FINISHED(28); sieve[i0] &= BIT4; i0 += sievingPrime * 2 + 1; FALLTHROUGH;
        case 29: CHECK_FINISHED(29); sieve[i0] &= BIT5; i0 += sievingPrime * 4 + 2; FALLTHROUGH;
        case 30: CHECK_FINISHED(30); sieve[i0] &= BIT7; i0 += sievingPrime * 6 + 4; FALLTHROUGH;
        case 31: CHECK_FINISHED(31); sieve[i0] &= BIT2; i0 += sievingPrime * 2 + 1;
      }

      // sievingPrime % 30 == 19
      for (;;)
      {
        case 32: {
                   std::size_t i1 = i0 + sievingPrime *  6 +  4;
                   std::size_t i2 = i0 + sievingPrime * 10 +  6;
                   std::size_t i3 = i0 + sievingPrime * 12 +  8;
                   std::size_t i4 = i0 + sievingPrime * 16 + 10;
                   std::size_t i5 = i0 + sievingPrime * 18 + 11;
                   std::size_t i6 = i0 + sievingPrime * 22 + 14;
                   std::size_t i7 = i0 + sievingPrime * 28 + 18;
                   std::size_t dist = sievingPrime * 30 + 19;

                   // Each iteration removes the next 8
                   // multiples of the sievingPrime.
                   while (i7 < sieveBytes)
                   {
                     sieve[i0] &= BIT4; i0 += dist;
                     sieve[i1] &= BIT2; i1 += dist;
                     sieve[i2] &= BIT6; i2 += dist;
                     sieve[i3] &= BIT0; i3 += dist;
                     sieve[i4] &= BIT5; i4 += dist;
                     sieve[i5] &= BIT7; i5 += dist;
                     sieve[i6] &= BIT3; i6 += dist;
                     sieve[i7] &= BIT1; i7 += dist;
                   }
                 }
                 CHECK_FINISHED(32); sieve[i0] &= BIT4; i0 += sievingPrime * 6 + 4; FALLTHROUGH;
        case 33: CHECK_FINISHED(33); sieve[i0] &= BIT2; i0 += sievingPrime * 4 + 2; FALLTHROUGH;
        case 34: CHECK_FINISHED(34); sieve[i0] &= BIT6; i0 += sievingPrime * 2 + 2; FALLTHROUGH;
        case 35: CHECK_FINISHED(35); sieve[i0] &= BIT0; i0 += sievingPrime * 4 + 2; FALLTHROUGH;
        case 36: CHECK_FINISHED(36); sieve[i0] &= BIT5; i0 += sievingPrime * 2 + 1; FALLTHROUGH;
        case 37: CHECK_FINISHED(37); sieve[i0] &= BIT7; i0 += sievingPrime * 4 + 3; FALLTHROUGH;
        case 38: CHECK_FINISHED(38); sieve[i0] &= BIT3; i0 += sievingPrime * 6 + 4; FALLTHROUGH;
        case 39: CHECK_FINISHED(39); sieve[i0] &= BIT1; i0 += sievingPrime * 2 + 1;
      }

      // sievingPrime % 30 == 23
      for (;;)
      {
        case 40: {
                   std::size_t i1 = i0 + sievingPrime *  6 +  5;
                   std::size_t i2 = i0 + sievingPrime * 10 +  8;
                   std::size_t i3 = i0 + sievingPrime * 12 +  9;
                   std::size_t i4 = i0 + sievingPrime * 16 + 12;
                   std::size_t i5 = i0 + sievingPrime * 18 + 14;
                   std::size_t i6 = i0 + sievingPrime * 22 + 17;
                   std::size_t i7 = i0 + sievingPrime * 28 + 22;
                   std::size_t dist = sievingPrime * 30 + 23;

                   // Each iteration removes the next 8
                   // multiples of the sievingPrime.
                   while (i7 < sieveBytes)
                   {
                     sieve[i0] &= BIT5; i0 += dist;
                     sieve[i1] &= BIT1; i1 += dist;
                     sieve[i2] &= BIT2; i2 += dist;
                     sieve[i3] &= BIT6; i3 += dist;
                     sieve[i4] &= BIT7; i4 += dist;
                     sieve[i5] &= BIT3; i5 += dist;
                     sieve[i6] &= BIT4; i6 += dist;
                     sieve[i7] &= BIT0; i7 += dist;
                   }
                 }
                 CHECK_FINISHED(40); sieve[i0] &= BIT5; i0 += sievingPrime * 6 + 5; FALLTHROUGH;
        case 41: CHECK_FINISHED(41); sieve[i0] &= BIT1; i0 += sievingPrime * 4 + 3; FALLTHROUGH;
        case 42: CHECK_FINISHED(42); sieve[i0] &= BIT2; i0 += sievingPrime * 2 + 1; FALLTHROUGH;
        case 43: CHECK_FINISHED(43); sieve[i0] &= BIT6; i0 += sievingPrime * 4 + 3; FALLTHROUGH;
        case 44: CHECK_FINISHED(44); sieve[i0] &= BIT7; i0 += sievingPrime * 2 + 2; FALLTHROUGH;
        case 45: CHECK_FINISHED(45); sieve[i0] &= BIT3; i0 += sievingPrime * 4 + 3; FALLTHROUGH;
        case 46: CHECK_FINISHED(46); sieve[i0] &= BIT4; i0 += sievingPrime * 6 + 5; FALLTHROUGH;
        case 47: CHECK_FINISHED(47); sieve[i0] &= BIT0; i0 += sievingPrime * 2 + 1;
      }

      // sievingPrime % 30 == 29
      for (;;)
      {
        case 48: {
                   std::size_t i1 = i0 + sievingPrime *  6 +  6;
                   std::size_t i2 = i0 + sievingPrime * 10 + 10;
                   std::size_t i3 = i0 + sievingPrime * 12 + 12;
                   std::size_t i4 = i0 + sievingPrime * 16 + 16;
                   std::size_t i5 = i0 + sievingPrime * 18 + 18;
                   std::size_t i6 = i0 + sievingPrime * 22 + 22;
                   std::size_t i7 = i0 + sievingPrime * 28 + 27;
                   std::size_t dist = sievingPrime * 30 + 29;

                   // Each iteration removes the next 8
                   // multiples of the sievingPrime.
                   while (i7 < sieveBytes)
                   {
                     sieve[i0] &= BIT6; i0 += dist;
                     sieve[i1] &= BIT5; i1 += dist;
                     sieve[i2] &= BIT4; i2 += dist;
                     sieve[i3] &= BIT3; i3 += dist;
                     sieve[i4] &= BIT2; i4 += dist;
                     sieve[i5] &= BIT1; i5 += dist;
                     sieve[i6] &= BIT0; i6 += dist;
                     sieve[i7] &= BIT7; i7 += dist;
                   }
                 }
                 CHECK_FINISHED(48); sieve[i0] &= BIT6; i0 += sievingPrime * 6 + 6; FALLTHROUGH;
        case 49: CHECK_FINISHED(49); sieve[i0] &= BIT5; i0 += sievingPrime * 4 + 4; FALLTHROUGH;
        case 50: CHECK_FINISHED(50); sieve[i0] &= BIT4; i0 += sievingPrime * 2 + 2; FALLTHROUGH;
        case 51: CHECK_FINISHED(51); sieve[i0] &= BIT3; i0 += sievingPrime * 4 + 4; FALLTHROUGH;
        case 52: CHECK_FINISHED(52); sieve[i0] &= BIT2; i0 += sievingPrime * 2 + 2; FALLTHROUGH;
        case 53: CHECK_FINISHED(53); sieve[i0] &= BIT1; i0 += sievingPrime * 4 + 4; FALLTHROUGH;
        case 54: CHECK_FINISHED(54); sieve[i0] &= BIT0; i0 += sievingPrime * 6 + 5; FALLTHROUGH;
        case 55: CHECK_FINISHED(55); sieve[i0] &= BIT7; i0 += sievingPrime * 2 + 2;
      }

      // sievingPrime % 30 == 1
      for (;;)
      {
        case 56: {
                   std::size_t i1 = i0 + sievingPrime *  6 + 1;
                   std::size_t i2 = i0 + sievingPrime * 10 + 1;
                   std::size_t i3 = i0 + sievingPrime * 12 + 1;
                   std::size_t i4 = i0 + sievingPrime * 16 + 1;
                   std::size_t i5 = i0 + sievingPrime * 18 + 1;
                   std::size_t i6 = i0 + sievingPrime * 22 + 1;
                   std::size_t i7 = i0 + sievingPrime * 28 + 1;
                   std::size_t dist = sievingPrime * 30 + 1;

                   // Each iteration removes the next 8
                   // multiples of the sievingPrime.
                   while (i7 < sieveBytes)
                   {
                     sieve[i0] &= BIT7; i0 += dist;
                     sieve[i1] &= BIT0; i1 += dist;
                     sieve[i2] &= BIT1; i2 += dist;
                     sieve[i3] &= BIT2; i3 += dist;
                     sieve[i4] &= BIT3; i4 += dist;
                     sieve[i5] &= BIT4; i5 += dist;
                     sieve[i6] &= BIT5; i6 += dist;
                     sieve[i7] &= BIT6; i7 += dist;
                   }
                 }
                 CHECK_FINISHED(56); sieve[i0] &= BIT7; i0 += sievingPrime * 6 + 1; FALLTHROUGH;
        case 57: CHECK_FINISHED(57); sieve[i0] &= BIT0; i0 += sievingPrime * 4 + 0; FALLTHROUGH;
        case 58: CHECK_FINISHED(58); sieve[i0] &= BIT1; i0 += sievingPrime * 2 + 0; FALLTHROUGH;
        case 59: CHECK_FINISHED(59); sieve[i0] &= BIT2; i0 += sievingPrime * 4 + 0; FALLTHROUGH;
        case 60: CHECK_FINISHED(60); sieve[i0] &= BIT3; i0 += sievingPrime * 2 + 0; FALLTHROUGH;
        case 61: CHECK_FINISHED(61); sieve[i0] &= BIT4; i0 += sievingPrime * 4 + 0; FALLTHROUGH;
        case 62: CHECK_FINISHED(62); sieve[i0] &= BIT5; i0 += sievingPrime * 6 + 0; FALLTHROUGH;
        case 63: CHECK_FINISHED(63); sieve[i0] &= BIT6; i0 += sievingPrime * 2 + 0;
      }

      default: UNREACHABLE;
    }

    next_iteration:;
  }
}

} // namespace
