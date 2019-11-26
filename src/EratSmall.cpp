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
/// Copyright (C) 2019 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/EratSmall.hpp>
#include <primesieve/bits.hpp>
#include <primesieve/Bucket.hpp>
#include <primesieve/CpuInfo.hpp>
#include <primesieve/pmath.hpp>
#include <primesieve/primesieve_error.hpp>
#include <primesieve/Wheel.hpp>

#include <stdint.h>
#include <algorithm>
#include <cassert>
#include <vector>

/// Update the current sieving prime's multipleIndex
/// and wheelIndex after sieving has finished.
///
#define UPDATE_SIEVING_PRIME(wheelIndex) \
  out ## wheelIndex: \
  multipleIndex = (uint64_t) (p - sieveEnd); \
  prime.set(multipleIndex, wheelIndex); \
  continue;

namespace primesieve {

/// @stop:        Upper bound for sieving
/// @l1CacheSize: CPU L1 cache size
/// @maxPrime:    Sieving primes <= maxPrime
///
void EratSmall::init(uint64_t stop, uint64_t l1CacheSize, uint64_t maxPrime)
{
  if (maxPrime > l1CacheSize * 3)
    throw primesieve_error("EratSmall: maxPrime > l1CacheSize * 3");

  enabled_ = true;
  maxPrime_ = maxPrime;
  l1CacheSize_ = l1CacheSize;
  Wheel::init(stop, l1CacheSize);

  size_t count = primeCountApprox(maxPrime);
  primes_.reserve(count);
}

/// Add a new sieving prime to EratSmall
void EratSmall::storeSievingPrime(uint64_t prime, uint64_t multipleIndex, uint64_t wheelIndex)
{
  assert(prime <= maxPrime_);
  uint64_t sievingPrime = prime / 30;
  primes_.emplace_back(sievingPrime, multipleIndex, wheelIndex);
}

/// Use the CPU's L1 cache size as
/// sieveSize in EratSmall.
///
uint64_t EratSmall::getL1CacheSize(uint64_t sieveSize)
{
  if (!cpuInfo.hasL1Cache())
    return sieveSize;

  uint64_t size = cpuInfo.l1CacheSize();
  uint64_t minSize = 8 << 10;
  uint64_t maxSize = 4096 << 10;

  size = std::min(size, sieveSize);
  size = inBetween(minSize, size, maxSize);

  return size;
}

/// Both EratMedium and EratBig run fastest using a sieve size
/// that matches the CPU's L2 cache size (or slightly less).
/// However, proportionally EratSmall does a lot more memory
/// writes than both EratMedium and EratBig and hence EratSmall
/// runs fastest using a smaller sieve size that matches the
/// CPU's L1 cache size.
///
/// @sieveSize:   CPU L2 cache size / 2
/// @l1CacheSize: CPU L1 cache size
///
void EratSmall::crossOff(uint8_t* sieve, uint64_t sieveSize)
{
  uint8_t* sieveEnd = sieve + sieveSize;

  while (sieve < sieveEnd)
  {
    uint8_t* start = sieve;
    sieve += l1CacheSize_;
    sieve = std::min(sieve, sieveEnd);
    crossOff(start, sieve);
  }
}

/// Segmented sieve of Eratosthenes with wheel factorization
/// optimized for small sieving primes that have many multiples
/// per segment. This algorithm uses a hardcoded modulo 30
/// wheel that skips multiples of 2, 3 and 5.
///
void EratSmall::crossOff(uint8_t* sieve, uint8_t* sieveEnd)
{
  for (auto& prime : primes_)
  {
    uint64_t sievingPrime = prime.getSievingPrime();
    uint64_t multipleIndex = prime.getMultipleIndex();
    uint64_t wheelIndex = prime.getWheelIndex();
    uint64_t maxLoopDist = sievingPrime * 28 + 27;

    // pointer to the byte containing the first multiple
    // of sievingPrime within the current segment
    uint8_t* p = &sieve[multipleIndex];
    uint8_t* loopEnd = sieveEnd - maxLoopDist;

    if (loopEnd > sieveEnd)
      loopEnd = p;

    switch (wheelIndex)
    {
      // sievingPrime % 30 == 7
      for (;;)
      {
        case 0: // each iteration removes the next 8 multiples
                // of the current sievingPrime
                for (; p < loopEnd; p += sievingPrime * 30 + 7)
                {
                  p[sievingPrime *  0 + 0] &= BIT0;
                  p[sievingPrime *  6 + 1] &= BIT4;
                  p[sievingPrime * 10 + 2] &= BIT3;
                  p[sievingPrime * 12 + 2] &= BIT7;
                  p[sievingPrime * 16 + 3] &= BIT6;
                  p[sievingPrime * 18 + 4] &= BIT2;
                  p[sievingPrime * 22 + 5] &= BIT1;
                  p[sievingPrime * 28 + 6] &= BIT5;
                }
                if (p >= sieveEnd) goto out0;
                *p &= BIT0; p += sievingPrime * 6 + 1;
        case 1: if (p >= sieveEnd) goto out1;
                *p &= BIT4; p += sievingPrime * 4 + 1;
        case 2: if (p >= sieveEnd) goto out2;
                *p &= BIT3; p += sievingPrime * 2 + 0;
        case 3: if (p >= sieveEnd) goto out3;
                *p &= BIT7; p += sievingPrime * 4 + 1;
        case 4: if (p >= sieveEnd) goto out4;
                *p &= BIT6; p += sievingPrime * 2 + 1;
        case 5: if (p >= sieveEnd) goto out5;
                *p &= BIT2; p += sievingPrime * 4 + 1;
        case 6: if (p >= sieveEnd) goto out6;
                *p &= BIT1; p += sievingPrime * 6 + 1;
        case 7: if (p >= sieveEnd) goto out7;
                *p &= BIT5; p += sievingPrime * 2 + 1;
      }

      UPDATE_SIEVING_PRIME(0)
      UPDATE_SIEVING_PRIME(1)
      UPDATE_SIEVING_PRIME(2)
      UPDATE_SIEVING_PRIME(3)
      UPDATE_SIEVING_PRIME(4)
      UPDATE_SIEVING_PRIME(5)
      UPDATE_SIEVING_PRIME(6)
      UPDATE_SIEVING_PRIME(7)

      // sievingPrime % 30 == 11
      for (;;)
      {
        case  8: for (; p < loopEnd; p += sievingPrime * 30 + 11)
                 {
                   p[sievingPrime *  0 +  0] &= BIT1;
                   p[sievingPrime *  6 +  2] &= BIT3;
                   p[sievingPrime * 10 +  3] &= BIT7;
                   p[sievingPrime * 12 +  4] &= BIT5;
                   p[sievingPrime * 16 +  6] &= BIT0;
                   p[sievingPrime * 18 +  6] &= BIT6;
                   p[sievingPrime * 22 +  8] &= BIT2;
                   p[sievingPrime * 28 + 10] &= BIT4;
                 }
                 if (p >= sieveEnd) goto out8;
                 *p &= BIT1; p += sievingPrime * 6 + 2;
        case  9: if (p >= sieveEnd) goto out9;
                 *p &= BIT3; p += sievingPrime * 4 + 1;
        case 10: if (p >= sieveEnd) goto out10;
                 *p &= BIT7; p += sievingPrime * 2 + 1;
        case 11: if (p >= sieveEnd) goto out11;
                 *p &= BIT5; p += sievingPrime * 4 + 2;
        case 12: if (p >= sieveEnd) goto out12;
                 *p &= BIT0; p += sievingPrime * 2 + 0;
        case 13: if (p >= sieveEnd) goto out13;
                 *p &= BIT6; p += sievingPrime * 4 + 2;
        case 14: if (p >= sieveEnd) goto out14;
                 *p &= BIT2; p += sievingPrime * 6 + 2;
        case 15: if (p >= sieveEnd) goto out15;
                 *p &= BIT4; p += sievingPrime * 2 + 1;
      }

      UPDATE_SIEVING_PRIME(8)
      UPDATE_SIEVING_PRIME(9)
      UPDATE_SIEVING_PRIME(10)
      UPDATE_SIEVING_PRIME(11)
      UPDATE_SIEVING_PRIME(12)
      UPDATE_SIEVING_PRIME(13)
      UPDATE_SIEVING_PRIME(14)
      UPDATE_SIEVING_PRIME(15)

      // sievingPrime % 30 == 13
      for (;;)
      {
        case 16: for (; p < loopEnd; p += sievingPrime * 30 + 13)
                 {
                   p[sievingPrime *  0 +  0] &= BIT2;
                   p[sievingPrime *  6 +  2] &= BIT7;
                   p[sievingPrime * 10 +  4] &= BIT5;
                   p[sievingPrime * 12 +  5] &= BIT4;
                   p[sievingPrime * 16 +  7] &= BIT1;
                   p[sievingPrime * 18 +  8] &= BIT0;
                   p[sievingPrime * 22 +  9] &= BIT6;
                   p[sievingPrime * 28 + 12] &= BIT3;
                 }
                 if (p >= sieveEnd) goto out16;
                 *p &= BIT2; p += sievingPrime * 6 + 2;
        case 17: if (p >= sieveEnd) goto out17;
                 *p &= BIT7; p += sievingPrime * 4 + 2;
        case 18: if (p >= sieveEnd) goto out18;
                 *p &= BIT5; p += sievingPrime * 2 + 1;
        case 19: if (p >= sieveEnd) goto out19;
                 *p &= BIT4; p += sievingPrime * 4 + 2;
        case 20: if (p >= sieveEnd) goto out20;
                 *p &= BIT1; p += sievingPrime * 2 + 1;
        case 21: if (p >= sieveEnd) goto out21;
                 *p &= BIT0; p += sievingPrime * 4 + 1;
        case 22: if (p >= sieveEnd) goto out22;
                 *p &= BIT6; p += sievingPrime * 6 + 3;
        case 23: if (p >= sieveEnd) goto out23;
                 *p &= BIT3; p += sievingPrime * 2 + 1;
      }

      UPDATE_SIEVING_PRIME(16)
      UPDATE_SIEVING_PRIME(17)
      UPDATE_SIEVING_PRIME(18)
      UPDATE_SIEVING_PRIME(19)
      UPDATE_SIEVING_PRIME(20)
      UPDATE_SIEVING_PRIME(21)
      UPDATE_SIEVING_PRIME(22)
      UPDATE_SIEVING_PRIME(23)

      // sievingPrime % 30 == 17
      for (;;)
      {
        case 24: for (; p < loopEnd; p += sievingPrime * 30 + 17)
                 {
                   p[sievingPrime *  0 +  0] &= BIT3;
                   p[sievingPrime *  6 +  3] &= BIT6;
                   p[sievingPrime * 10 +  6] &= BIT0;
                   p[sievingPrime * 12 +  7] &= BIT1;
                   p[sievingPrime * 16 +  9] &= BIT4;
                   p[sievingPrime * 18 + 10] &= BIT5;
                   p[sievingPrime * 22 + 12] &= BIT7;
                   p[sievingPrime * 28 + 16] &= BIT2;
                 }
                 if (p >= sieveEnd) goto out24;
                 *p &= BIT3; p += sievingPrime * 6 + 3;
        case 25: if (p >= sieveEnd) goto out25;
                 *p &= BIT6; p += sievingPrime * 4 + 3;
        case 26: if (p >= sieveEnd) goto out26;
                 *p &= BIT0; p += sievingPrime * 2 + 1;
        case 27: if (p >= sieveEnd) goto out27;
                 *p &= BIT1; p += sievingPrime * 4 + 2;
        case 28: if (p >= sieveEnd) goto out28;
                 *p &= BIT4; p += sievingPrime * 2 + 1;
        case 29: if (p >= sieveEnd) goto out29;
                 *p &= BIT5; p += sievingPrime * 4 + 2;
        case 30: if (p >= sieveEnd) goto out30;
                 *p &= BIT7; p += sievingPrime * 6 + 4;
        case 31: if (p >= sieveEnd) goto out31;
                 *p &= BIT2; p += sievingPrime * 2 + 1;
      }

      UPDATE_SIEVING_PRIME(24)
      UPDATE_SIEVING_PRIME(25)
      UPDATE_SIEVING_PRIME(26)
      UPDATE_SIEVING_PRIME(27)
      UPDATE_SIEVING_PRIME(28)
      UPDATE_SIEVING_PRIME(29)
      UPDATE_SIEVING_PRIME(30)
      UPDATE_SIEVING_PRIME(31)

      // sievingPrime % 30 == 19
      for (;;)
      {
        case 32: for (; p < loopEnd; p += sievingPrime * 30 + 19)
                 {
                   p[sievingPrime *  0 +  0] &= BIT4;
                   p[sievingPrime *  6 +  4] &= BIT2;
                   p[sievingPrime * 10 +  6] &= BIT6;
                   p[sievingPrime * 12 +  8] &= BIT0;
                   p[sievingPrime * 16 + 10] &= BIT5;
                   p[sievingPrime * 18 + 11] &= BIT7;
                   p[sievingPrime * 22 + 14] &= BIT3;
                   p[sievingPrime * 28 + 18] &= BIT1;
                 }
                 if (p >= sieveEnd) goto out32;
                 *p &= BIT4; p += sievingPrime * 6 + 4;
        case 33: if (p >= sieveEnd) goto out33;
                 *p &= BIT2; p += sievingPrime * 4 + 2;
        case 34: if (p >= sieveEnd) goto out34;
                 *p &= BIT6; p += sievingPrime * 2 + 2;
        case 35: if (p >= sieveEnd) goto out35;
                 *p &= BIT0; p += sievingPrime * 4 + 2;
        case 36: if (p >= sieveEnd) goto out36;
                 *p &= BIT5; p += sievingPrime * 2 + 1;
        case 37: if (p >= sieveEnd) goto out37;
                 *p &= BIT7; p += sievingPrime * 4 + 3;
        case 38: if (p >= sieveEnd) goto out38;
                 *p &= BIT3; p += sievingPrime * 6 + 4;
        case 39: if (p >= sieveEnd) goto out39;
                 *p &= BIT1; p += sievingPrime * 2 + 1;
      }

      UPDATE_SIEVING_PRIME(32)
      UPDATE_SIEVING_PRIME(33)
      UPDATE_SIEVING_PRIME(34)
      UPDATE_SIEVING_PRIME(35)
      UPDATE_SIEVING_PRIME(36)
      UPDATE_SIEVING_PRIME(37)
      UPDATE_SIEVING_PRIME(38)
      UPDATE_SIEVING_PRIME(39)

      // sievingPrime % 30 == 23
      for (;;)
      {
        case 40: for (; p < loopEnd; p += sievingPrime * 30 + 23)
                 {
                   p[sievingPrime *  0 +  0] &= BIT5;
                   p[sievingPrime *  6 +  5] &= BIT1;
                   p[sievingPrime * 10 +  8] &= BIT2;
                   p[sievingPrime * 12 +  9] &= BIT6;
                   p[sievingPrime * 16 + 12] &= BIT7;
                   p[sievingPrime * 18 + 14] &= BIT3;
                   p[sievingPrime * 22 + 17] &= BIT4;
                   p[sievingPrime * 28 + 22] &= BIT0;
                 }
                 if (p >= sieveEnd) goto out40;
                 *p &= BIT5; p += sievingPrime * 6 + 5;
        case 41: if (p >= sieveEnd) goto out41;
                 *p &= BIT1; p += sievingPrime * 4 + 3;
        case 42: if (p >= sieveEnd) goto out42;
                 *p &= BIT2; p += sievingPrime * 2 + 1;
        case 43: if (p >= sieveEnd) goto out43;
                 *p &= BIT6; p += sievingPrime * 4 + 3;
        case 44: if (p >= sieveEnd) goto out44;
                 *p &= BIT7; p += sievingPrime * 2 + 2;
        case 45: if (p >= sieveEnd) goto out45;
                 *p &= BIT3; p += sievingPrime * 4 + 3;
        case 46: if (p >= sieveEnd) goto out46;
                 *p &= BIT4; p += sievingPrime * 6 + 5;
        case 47: if (p >= sieveEnd) goto out47;
                 *p &= BIT0; p += sievingPrime * 2 + 1;
      }

      UPDATE_SIEVING_PRIME(40)
      UPDATE_SIEVING_PRIME(41)
      UPDATE_SIEVING_PRIME(42)
      UPDATE_SIEVING_PRIME(43)
      UPDATE_SIEVING_PRIME(44)
      UPDATE_SIEVING_PRIME(45)
      UPDATE_SIEVING_PRIME(46)
      UPDATE_SIEVING_PRIME(47)

      // sievingPrime % 30 == 29
      for (;;)
      {
        case 48: for (; p < loopEnd; p += sievingPrime * 30 + 29)
                 {
                   p[sievingPrime *  0 +  0] &= BIT6;
                   p[sievingPrime *  6 +  6] &= BIT5;
                   p[sievingPrime * 10 + 10] &= BIT4;
                   p[sievingPrime * 12 + 12] &= BIT3;
                   p[sievingPrime * 16 + 16] &= BIT2;
                   p[sievingPrime * 18 + 18] &= BIT1;
                   p[sievingPrime * 22 + 22] &= BIT0;
                   p[sievingPrime * 28 + 27] &= BIT7;
                 }
                 if (p >= sieveEnd) goto out48;
                 *p &= BIT6; p += sievingPrime * 6 + 6;
        case 49: if (p >= sieveEnd) goto out49;
                 *p &= BIT5; p += sievingPrime * 4 + 4;
        case 50: if (p >= sieveEnd) goto out50;
                 *p &= BIT4; p += sievingPrime * 2 + 2;
        case 51: if (p >= sieveEnd) goto out51;
                 *p &= BIT3; p += sievingPrime * 4 + 4;
        case 52: if (p >= sieveEnd) goto out52;
                 *p &= BIT2; p += sievingPrime * 2 + 2;
        case 53: if (p >= sieveEnd) goto out53;
                 *p &= BIT1; p += sievingPrime * 4 + 4;
        case 54: if (p >= sieveEnd) goto out54;
                 *p &= BIT0; p += sievingPrime * 6 + 5;
        case 55: if (p >= sieveEnd) goto out55;
                 *p &= BIT7; p += sievingPrime * 2 + 2;
      }

      UPDATE_SIEVING_PRIME(48)
      UPDATE_SIEVING_PRIME(49)
      UPDATE_SIEVING_PRIME(50)
      UPDATE_SIEVING_PRIME(51)
      UPDATE_SIEVING_PRIME(52)
      UPDATE_SIEVING_PRIME(53)
      UPDATE_SIEVING_PRIME(54)
      UPDATE_SIEVING_PRIME(55)

      // sievingPrime % 30 == 1
      for (;;)
      {
        case 56: for (; p < loopEnd; p += sievingPrime * 30 + 1)
                 {
                   p[sievingPrime *  0 + 0] &= BIT7;
                   p[sievingPrime *  6 + 1] &= BIT0;
                   p[sievingPrime * 10 + 1] &= BIT1;
                   p[sievingPrime * 12 + 1] &= BIT2;
                   p[sievingPrime * 16 + 1] &= BIT3;
                   p[sievingPrime * 18 + 1] &= BIT4;
                   p[sievingPrime * 22 + 1] &= BIT5;
                   p[sievingPrime * 28 + 1] &= BIT6;
                 }
                 if (p >= sieveEnd) goto out56;
                 *p &= BIT7; p += sievingPrime * 6 + 1;
        case 57: if (p >= sieveEnd) goto out57;
                 *p &= BIT0; p += sievingPrime * 4 + 0;
        case 58: if (p >= sieveEnd) goto out58;
                 *p &= BIT1; p += sievingPrime * 2 + 0;
        case 59: if (p >= sieveEnd) goto out59;
                 *p &= BIT2; p += sievingPrime * 4 + 0;
        case 60: if (p >= sieveEnd) goto out60;
                 *p &= BIT3; p += sievingPrime * 2 + 0;
        case 61: if (p >= sieveEnd) goto out61;
                 *p &= BIT4; p += sievingPrime * 4 + 0;
        case 62: if (p >= sieveEnd) goto out62;
                 *p &= BIT5; p += sievingPrime * 6 + 0;
        case 63: if (p >= sieveEnd) goto out63;
                 *p &= BIT6; p += sievingPrime * 2 + 0;
      }

      UPDATE_SIEVING_PRIME(56)
      UPDATE_SIEVING_PRIME(57)
      UPDATE_SIEVING_PRIME(58)
      UPDATE_SIEVING_PRIME(59)
      UPDATE_SIEVING_PRIME(60)
      UPDATE_SIEVING_PRIME(61)
      UPDATE_SIEVING_PRIME(62)
      UPDATE_SIEVING_PRIME(63)
    }
  }
}

} // namespace
