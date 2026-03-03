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

const uint8_t wheel30UnsetBit[64] =
{
  BIT0, BIT4, BIT3, BIT7, BIT6, BIT2, BIT1, BIT5,
  BIT1, BIT3, BIT7, BIT5, BIT0, BIT6, BIT2, BIT4,
  BIT2, BIT7, BIT5, BIT4, BIT1, BIT0, BIT6, BIT3,
  BIT3, BIT6, BIT0, BIT1, BIT4, BIT5, BIT7, BIT2,
  BIT4, BIT2, BIT6, BIT0, BIT5, BIT7, BIT3, BIT1,
  BIT5, BIT1, BIT2, BIT6, BIT7, BIT3, BIT4, BIT0,
  BIT6, BIT5, BIT4, BIT3, BIT2, BIT1, BIT0, BIT7,
  BIT7, BIT0, BIT1, BIT2, BIT3, BIT4, BIT5, BIT6
};

const uint8_t wheel30DistMul[64] =
{
  6, 4, 2, 4, 2, 4, 6, 2,
  6, 4, 2, 4, 2, 4, 6, 2,
  6, 4, 2, 4, 2, 4, 6, 2,
  6, 4, 2, 4, 2, 4, 6, 2,
  6, 4, 2, 4, 2, 4, 6, 2,
  6, 4, 2, 4, 2, 4, 6, 2,
  6, 4, 2, 4, 2, 4, 6, 2,
  6, 4, 2, 4, 2, 4, 6, 2
};

const uint8_t wheel30DistAdd[64] =
{
  1, 1, 0, 1, 1, 1, 1, 1,
  2, 1, 1, 2, 0, 2, 2, 1,
  2, 2, 1, 2, 1, 1, 3, 1,
  3, 3, 1, 2, 1, 2, 4, 1,
  4, 2, 2, 2, 1, 3, 4, 1,
  5, 3, 1, 3, 2, 3, 5, 1,
  6, 4, 2, 4, 2, 4, 5, 2,
  1, 0, 0, 0, 0, 0, 0, 0
};

const uint8_t wheel30MaxOffsetAdd[8] =
{
   6, 10, 12, 16, 18, 22, 27, 1
};

const uint8_t wheel30LoopAdd[8] =
{
   7, 11, 13, 17, 19, 23, 29, 1
};

const uint8_t wheel30LoopOffsetAdd[8][8] =
{
  { 0,  1,  2,  2,  3,  4,  5,  6 },
  { 0,  2,  3,  4,  6,  6,  8, 10 },
  { 0,  2,  4,  5,  7,  8,  9, 12 },
  { 0,  3,  6,  7,  9, 10, 12, 16 },
  { 0,  4,  6,  8, 10, 11, 14, 18 },
  { 0,  5,  8,  9, 12, 14, 17, 22 },
  { 0,  6, 10, 12, 16, 18, 22, 27 },
  { 0,  1,  1,  1,  1,  1,  1,  1 }
};

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
    std::size_t base = wheelIndex & ~std::size_t(7);
    std::size_t state = wheelIndex & 7;
    std::size_t group = wheelIndex >> 3;
    std::size_t maxOffset = sievingPrime * 28 + wheel30MaxOffsetAdd[group];
    std::size_t limit = std::max(sieveSize, maxOffset) - maxOffset;
    std::size_t loopDist = sievingPrime * 30 + wheel30LoopAdd[group];
    std::size_t adv[8] =
    {
      sievingPrime * wheel30DistMul[base + 0] + wheel30DistAdd[base + 0],
      sievingPrime * wheel30DistMul[base + 1] + wheel30DistAdd[base + 1],
      sievingPrime * wheel30DistMul[base + 2] + wheel30DistAdd[base + 2],
      sievingPrime * wheel30DistMul[base + 3] + wheel30DistAdd[base + 3],
      sievingPrime * wheel30DistMul[base + 4] + wheel30DistAdd[base + 4],
      sievingPrime * wheel30DistMul[base + 5] + wheel30DistAdd[base + 5],
      sievingPrime * wheel30DistMul[base + 6] + wheel30DistAdd[base + 6],
      sievingPrime * wheel30DistMul[base + 7] + wheel30DistAdd[base + 7]
    };
    const uint8_t* unsetBit = &wheel30UnsetBit[base];
    std::size_t s0 = sievingPrime *  0 + wheel30LoopOffsetAdd[group][0];
    std::size_t s1 = sievingPrime *  6 + wheel30LoopOffsetAdd[group][1];
    std::size_t s2 = sievingPrime * 10 + wheel30LoopOffsetAdd[group][2];
    std::size_t s3 = sievingPrime * 12 + wheel30LoopOffsetAdd[group][3];
    std::size_t s4 = sievingPrime * 16 + wheel30LoopOffsetAdd[group][4];
    std::size_t s5 = sievingPrime * 18 + wheel30LoopOffsetAdd[group][5];
    std::size_t s6 = sievingPrime * 22 + wheel30LoopOffsetAdd[group][6];
    std::size_t s7 = sievingPrime * 28 + wheel30LoopOffsetAdd[group][7];

    // Get ready for loop unrolling.
    for (; state; state = (state + 1) & 7)
    {
      wheelIndex = base + state;
      CHECK_FINISHED(wheelIndex);
      sieve[i] &= unsetBit[state];
      i += adv[state];
    }

    // Each iteration removes the next 8
    // multiples of the sievingPrime.
    for (; i < limit; i += loopDist)
    {
      sieve[i + s0] &= unsetBit[0];
      sieve[i + s1] &= unsetBit[1];
      sieve[i + s2] &= unsetBit[2];
      sieve[i + s3] &= unsetBit[3];
      sieve[i + s4] &= unsetBit[4];
      sieve[i + s5] &= unsetBit[5];
      sieve[i + s6] &= unsetBit[6];
      sieve[i + s7] &= unsetBit[7];
    }

    // Cross off the last few multiples where i >= limit.
    for (;; state = (state + 1) & 7)
    {
      wheelIndex = base + state;
      CHECK_FINISHED(wheelIndex);
      sieve[i] &= unsetBit[state];
      i += adv[state];
    }

    next_iteration:;
  }
}

} // namespace
