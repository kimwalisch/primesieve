///
/// @file   EratMedium.cpp
/// @brief  EratMedium is a segmented sieve of Eratosthenes
///         implementation optimized for medium sieving primes.
///         EratMedium is similar to EratSmall except that in
///         EratMedium each sieving prime is sorted (by its
///         wheelIndex) after the sieving step. When we then iterate
///         over the sorted sieving primes in the next segment the
///         initial indirect branch i.e. 'switch (wheelIndex)' is
///         predicted correctly by the CPU. This improves performance
///         by up to 30% for sieving primes that have only a few
///         multiple occurrences per segment.
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include "EratMedium.hpp"
#include "Bucket.hpp"
#include "MemoryPool.hpp"

#include <primesieve/bits.hpp>
#include <primesieve/macros.hpp>

#include <stdint.h>

namespace primesieve {

/// @stop:      Upper bound for sieving
/// @maxPrime:  Sieving primes <= maxPrime
///
void EratMedium::init(uint64_t stop,
                      uint64_t maxPrime,
                      MemoryPool& memoryPool)
{
  ASSERT((maxPrime / 30) * getMaxFactor() + getMaxFactor() <= SievingPrime::MAX_MULTIPLEINDEX);
  static_assert(config::FACTOR_ERATMEDIUM <= 4.5,
               "config::FACTOR_ERATMEDIUM > 4.5 causes multipleIndex overflow 23-bits!");

  stop_ = stop;
  maxPrime_ = maxPrime;
  memoryPool_ = &memoryPool;
}

/// Add a new sieving prime to EratMedium
void EratMedium::storeSievingPrime(uint64_t prime,
                                   uint64_t multipleIndex,
                                   uint64_t wheelIndex)
{
  ASSERT(prime <= maxPrime_);
  uint64_t sievingPrime = prime / 30;

  if_unlikely(buckets_.empty())
  {
    buckets_.resize(64);
    currentBuckets_.resize(64);
    std::fill_n(buckets_.begin(), 64, nullptr);
    std::fill_n(currentBuckets_.begin(), 64, nullptr);
  }

  if (Bucket::isFull(buckets_[wheelIndex]))
    memoryPool_->addBucket(buckets_[wheelIndex]);

  buckets_[wheelIndex]++->set(sievingPrime, multipleIndex, wheelIndex);
}

void EratMedium::crossOff(Vector<uint8_t>& sieve)
{
  currentBuckets_.swap(buckets_);

  // Iterate over the 64 bucket lists.
  // The 1st list contains sieving primes with wheelIndex = 0.
  // The 2nd list contains sieving primes with wheelIndex = 1.
  // The 3rd list contains sieving primes with wheelIndex = 2.
  // ...
  for (std::size_t i = 0; i < 64; i++)
  {
    if (currentBuckets_[i])
    {
      Bucket* bucket = Bucket::get(currentBuckets_[i]);
      bucket->setEnd(currentBuckets_[i]);
      currentBuckets_[i] = nullptr;
      std::size_t wheelIndex = i;

      // Iterate over the current bucket list.
      // For each bucket cross off the
      // multiples of its sieving primes.
      while (bucket)
      {
        switch (wheelIndex / 8)
        {
          case 0: crossOff_7 (sieve.data(), sieve.size(), bucket); break;
          case 1: crossOff_11(sieve.data(), sieve.size(), bucket); break;
          case 2: crossOff_13(sieve.data(), sieve.size(), bucket); break;
          case 3: crossOff_17(sieve.data(), sieve.size(), bucket); break;
          case 4: crossOff_19(sieve.data(), sieve.size(), bucket); break;
          case 5: crossOff_23(sieve.data(), sieve.size(), bucket); break;
          case 6: crossOff_29(sieve.data(), sieve.size(), bucket); break;
          case 7: crossOff_31(sieve.data(), sieve.size(), bucket); break;
          default: UNREACHABLE;
        }

        Bucket* processed = bucket;
        bucket = bucket->next();
        memoryPool_->freeBucket(processed);
      }
    }
  }
}

/// This macro sorts the current sieving prime by its
/// wheelIndex after sieving has finished. When we then
/// iterate over the sieving primes in the next segment the
/// 'switch (wheelIndex)' branch will be predicted
/// correctly by the CPU.
///
#define CHECK_FINISHED(wheelIndex) \
  if_unlikely(i >= sieveSize) \
  { \
    i -= sieveSize; \
    if (Bucket::isFull(buckets[wheelIndex])) \
      memoryPool.addBucket(buckets[wheelIndex]); \
    buckets[wheelIndex]++->set(sievingPrime, i, wheelIndex); \
    break; \
  }

/// For sieving primes of type n % 30 == 7
void EratMedium::crossOff_7(uint8_t* sieve, std::size_t sieveSize, Bucket* bucket)
{
  auto buckets = buckets_.data();
  MemoryPool& memoryPool = *memoryPool_;
  SievingPrime* prime = bucket->begin();
  SievingPrime* end = bucket->end();
  std::size_t wheelIndex = prime->getWheelIndex();

  for (; prime != end; prime++)
  {
    std::size_t sievingPrime = prime->getSievingPrime();
    std::size_t i = prime->getMultipleIndex();
    std::size_t dist0 = sievingPrime * 6 + 1;
    std::size_t dist1 = sievingPrime * 4 + 1;
    std::size_t dist2 = sievingPrime * 2 + 0;
    std::size_t dist4 = sievingPrime * 2 + 1;
    ASSERT(wheelIndex <= 7);

    switch (wheelIndex)
    {
      default: UNREACHABLE;

      for (;;)
      {
        case 0: CHECK_FINISHED(0); sieve[i] &= BIT0; i += dist0; FALLTHROUGH;
        case 1: CHECK_FINISHED(1); sieve[i] &= BIT4; i += dist1; FALLTHROUGH;
        case 2: CHECK_FINISHED(2); sieve[i] &= BIT3; i += dist2; FALLTHROUGH;
        case 3: CHECK_FINISHED(3); sieve[i] &= BIT7; i += dist1; FALLTHROUGH;
        case 4: CHECK_FINISHED(4); sieve[i] &= BIT6; i += dist4; FALLTHROUGH;
        case 5: CHECK_FINISHED(5); sieve[i] &= BIT2; i += dist1; FALLTHROUGH;
        case 6: CHECK_FINISHED(6); sieve[i] &= BIT1; i += dist0; FALLTHROUGH;
        case 7: CHECK_FINISHED(7); sieve[i] &= BIT5; i += dist4;
      }
    }
  }
}

/// For sieving primes of type n % 30 == 11
void EratMedium::crossOff_11(uint8_t* sieve, std::size_t sieveSize, Bucket* bucket)
{
  auto buckets = buckets_.data();
  MemoryPool& memoryPool = *memoryPool_;
  SievingPrime* prime = bucket->begin();
  SievingPrime* end = bucket->end();
  std::size_t wheelIndex = prime->getWheelIndex();

  for (; prime != end; prime++)
  {
    std::size_t sievingPrime = prime->getSievingPrime();
    std::size_t i = prime->getMultipleIndex();
    std::size_t dist0 = sievingPrime * 6 + 2;
    std::size_t dist1 = sievingPrime * 4 + 1;
    std::size_t dist2 = sievingPrime * 2 + 1;
    std::size_t dist3 = sievingPrime * 4 + 2;
    std::size_t dist4 = sievingPrime * 2 + 0;

    ASSERT(wheelIndex >= 8);
    ASSERT(wheelIndex <= 15);

    switch (wheelIndex)
    {
      default: UNREACHABLE;

      for (;;)
      {
        case  8: CHECK_FINISHED( 8); sieve[i] &= BIT1; i += dist0; FALLTHROUGH;
        case  9: CHECK_FINISHED( 9); sieve[i] &= BIT3; i += dist1; FALLTHROUGH;
        case 10: CHECK_FINISHED(10); sieve[i] &= BIT7; i += dist2; FALLTHROUGH;
        case 11: CHECK_FINISHED(11); sieve[i] &= BIT5; i += dist3; FALLTHROUGH;
        case 12: CHECK_FINISHED(12); sieve[i] &= BIT0; i += dist4; FALLTHROUGH;
        case 13: CHECK_FINISHED(13); sieve[i] &= BIT6; i += dist3; FALLTHROUGH;
        case 14: CHECK_FINISHED(14); sieve[i] &= BIT2; i += dist0; FALLTHROUGH;
        case 15: CHECK_FINISHED(15); sieve[i] &= BIT4; i += dist2;
      }
    }
  }
}

/// For sieving primes of type n % 30 == 13
void EratMedium::crossOff_13(uint8_t* sieve, std::size_t sieveSize, Bucket* bucket)
{
  auto buckets = buckets_.data();
  MemoryPool& memoryPool = *memoryPool_;
  SievingPrime* prime = bucket->begin();
  SievingPrime* end = bucket->end();
  std::size_t wheelIndex = prime->getWheelIndex();

  for (; prime != end; prime++)
  {
    std::size_t sievingPrime = prime->getSievingPrime();
    std::size_t i = prime->getMultipleIndex();
    std::size_t dist0 = sievingPrime * 6 + 2;
    std::size_t dist1 = sievingPrime * 4 + 2;
    std::size_t dist2 = sievingPrime * 2 + 1;
    std::size_t dist5 = sievingPrime * 4 + 1;
    std::size_t dist6 = sievingPrime * 6 + 3;

    ASSERT(wheelIndex >= 16);
    ASSERT(wheelIndex <= 23);

    switch (wheelIndex)
    {
      default: UNREACHABLE;

      for (;;)
      {
        case 16: CHECK_FINISHED(16); sieve[i] &= BIT2; i += dist0; FALLTHROUGH;
        case 17: CHECK_FINISHED(17); sieve[i] &= BIT7; i += dist1; FALLTHROUGH;
        case 18: CHECK_FINISHED(18); sieve[i] &= BIT5; i += dist2; FALLTHROUGH;
        case 19: CHECK_FINISHED(19); sieve[i] &= BIT4; i += dist1; FALLTHROUGH;
        case 20: CHECK_FINISHED(20); sieve[i] &= BIT1; i += dist2; FALLTHROUGH;
        case 21: CHECK_FINISHED(21); sieve[i] &= BIT0; i += dist5; FALLTHROUGH;
        case 22: CHECK_FINISHED(22); sieve[i] &= BIT6; i += dist6; FALLTHROUGH;
        case 23: CHECK_FINISHED(23); sieve[i] &= BIT3; i += dist2;
      }
    }
  }
}

/// For sieving primes of type n % 30 == 17
void EratMedium::crossOff_17(uint8_t* sieve, std::size_t sieveSize, Bucket* bucket)
{
  auto buckets = buckets_.data();
  MemoryPool& memoryPool = *memoryPool_;
  SievingPrime* prime = bucket->begin();
  SievingPrime* end = bucket->end();
  std::size_t wheelIndex = prime->getWheelIndex();

  for (; prime != end; prime++)
  {
    std::size_t sievingPrime = prime->getSievingPrime();
    std::size_t i = prime->getMultipleIndex();
    std::size_t dist0 = sievingPrime * 6 + 3;
    std::size_t dist1 = sievingPrime * 4 + 3;
    std::size_t dist2 = sievingPrime * 2 + 1;
    std::size_t dist3 = sievingPrime * 4 + 2;
    std::size_t dist6 = sievingPrime * 6 + 4;

    ASSERT(wheelIndex >= 24);
    ASSERT(wheelIndex <= 31);

    switch (wheelIndex)
    {
      default: UNREACHABLE;

      for (;;)
      {
        case 24: CHECK_FINISHED(24); sieve[i] &= BIT3; i += dist0; FALLTHROUGH;
        case 25: CHECK_FINISHED(25); sieve[i] &= BIT6; i += dist1; FALLTHROUGH;
        case 26: CHECK_FINISHED(26); sieve[i] &= BIT0; i += dist2; FALLTHROUGH;
        case 27: CHECK_FINISHED(27); sieve[i] &= BIT1; i += dist3; FALLTHROUGH;
        case 28: CHECK_FINISHED(28); sieve[i] &= BIT4; i += dist2; FALLTHROUGH;
        case 29: CHECK_FINISHED(29); sieve[i] &= BIT5; i += dist3; FALLTHROUGH;
        case 30: CHECK_FINISHED(30); sieve[i] &= BIT7; i += dist6; FALLTHROUGH;
        case 31: CHECK_FINISHED(31); sieve[i] &= BIT2; i += dist2;
      }
    }
  }
}

/// For sieving primes of type n % 30 == 19
void EratMedium::crossOff_19(uint8_t* sieve, std::size_t sieveSize, Bucket* bucket)
{
  auto buckets = buckets_.data();
  MemoryPool& memoryPool = *memoryPool_;
  SievingPrime* prime = bucket->begin();
  SievingPrime* end = bucket->end();
  std::size_t wheelIndex = prime->getWheelIndex();

  for (; prime != end; prime++)
  {
    std::size_t sievingPrime = prime->getSievingPrime();
    std::size_t i = prime->getMultipleIndex();
    std::size_t dist0 = sievingPrime * 6 + 4;
    std::size_t dist1 = sievingPrime * 4 + 2;
    std::size_t dist2 = sievingPrime * 2 + 2;
    std::size_t dist4 = sievingPrime * 2 + 1;
    std::size_t dist5 = sievingPrime * 4 + 3;

    ASSERT(wheelIndex >= 32);
    ASSERT(wheelIndex <= 39);

    switch (wheelIndex)
    {
      default: UNREACHABLE;

      for (;;)
      {
        case 32: CHECK_FINISHED(32); sieve[i] &= BIT4; i += dist0; FALLTHROUGH;
        case 33: CHECK_FINISHED(33); sieve[i] &= BIT2; i += dist1; FALLTHROUGH;
        case 34: CHECK_FINISHED(34); sieve[i] &= BIT6; i += dist2; FALLTHROUGH;
        case 35: CHECK_FINISHED(35); sieve[i] &= BIT0; i += dist1; FALLTHROUGH;
        case 36: CHECK_FINISHED(36); sieve[i] &= BIT5; i += dist4; FALLTHROUGH;
        case 37: CHECK_FINISHED(37); sieve[i] &= BIT7; i += dist5; FALLTHROUGH;
        case 38: CHECK_FINISHED(38); sieve[i] &= BIT3; i += dist0; FALLTHROUGH;
        case 39: CHECK_FINISHED(39); sieve[i] &= BIT1; i += dist4;
      }
    }
  }
}

/// For sieving primes of type n % 30 == 23
void EratMedium::crossOff_23(uint8_t* sieve, std::size_t sieveSize, Bucket* bucket)
{
  auto buckets = buckets_.data();
  MemoryPool& memoryPool = *memoryPool_;
  SievingPrime* prime = bucket->begin();
  SievingPrime* end = bucket->end();
  std::size_t wheelIndex = prime->getWheelIndex();

  for (; prime != end; prime++)
  {
    std::size_t sievingPrime = prime->getSievingPrime();
    std::size_t i = prime->getMultipleIndex();
    std::size_t dist0 = sievingPrime * 6 + 5;
    std::size_t dist1 = sievingPrime * 4 + 3;
    std::size_t dist2 = sievingPrime * 2 + 1;
    std::size_t dist4 = sievingPrime * 2 + 2;

    ASSERT(wheelIndex >= 40);
    ASSERT(wheelIndex <= 47);

    switch (wheelIndex)
    {
      default: UNREACHABLE;

      for (;;)
      {
        case 40: CHECK_FINISHED(40); sieve[i] &= BIT5; i += dist0; FALLTHROUGH;
        case 41: CHECK_FINISHED(41); sieve[i] &= BIT1; i += dist1; FALLTHROUGH;
        case 42: CHECK_FINISHED(42); sieve[i] &= BIT2; i += dist2; FALLTHROUGH;
        case 43: CHECK_FINISHED(43); sieve[i] &= BIT6; i += dist1; FALLTHROUGH;
        case 44: CHECK_FINISHED(44); sieve[i] &= BIT7; i += dist4; FALLTHROUGH;
        case 45: CHECK_FINISHED(45); sieve[i] &= BIT3; i += dist1; FALLTHROUGH;
        case 46: CHECK_FINISHED(46); sieve[i] &= BIT4; i += dist0; FALLTHROUGH;
        case 47: CHECK_FINISHED(47); sieve[i] &= BIT0; i += dist2;
      }
    }
  }
}

/// For sieving primes of type n % 30 == 29
void EratMedium::crossOff_29(uint8_t* sieve, std::size_t sieveSize, Bucket* bucket)
{
  auto buckets = buckets_.data();
  MemoryPool& memoryPool = *memoryPool_;
  SievingPrime* prime = bucket->begin();
  SievingPrime* end = bucket->end();
  std::size_t wheelIndex = prime->getWheelIndex();

  for (; prime != end; prime++)
  {
    std::size_t sievingPrime = prime->getSievingPrime();
    std::size_t i = prime->getMultipleIndex();
    std::size_t dist0 = sievingPrime * 6 + 6;
    std::size_t dist1 = sievingPrime * 4 + 4;
    std::size_t dist2 = sievingPrime * 2 + 2;
    std::size_t dist6 = sievingPrime * 6 + 5;

    ASSERT(wheelIndex >= 48);
    ASSERT(wheelIndex <= 55);

    switch (wheelIndex)
    {
      default: UNREACHABLE;

      for (;;)
      {
        case 48: CHECK_FINISHED(48); sieve[i] &= BIT6; i += dist0; FALLTHROUGH;
        case 49: CHECK_FINISHED(49); sieve[i] &= BIT5; i += dist1; FALLTHROUGH;
        case 50: CHECK_FINISHED(50); sieve[i] &= BIT4; i += dist2; FALLTHROUGH;
        case 51: CHECK_FINISHED(51); sieve[i] &= BIT3; i += dist1; FALLTHROUGH;
        case 52: CHECK_FINISHED(52); sieve[i] &= BIT2; i += dist2; FALLTHROUGH;
        case 53: CHECK_FINISHED(53); sieve[i] &= BIT1; i += dist1; FALLTHROUGH;
        case 54: CHECK_FINISHED(54); sieve[i] &= BIT0; i += dist6; FALLTHROUGH;
        case 55: CHECK_FINISHED(55); sieve[i] &= BIT7; i += dist2;
      }
    }
  }
}

/// For sieving primes of type n % 30 == 1
void EratMedium::crossOff_31(uint8_t* sieve, std::size_t sieveSize, Bucket* bucket)
{
  auto buckets = buckets_.data();
  MemoryPool& memoryPool = *memoryPool_;
  SievingPrime* prime = bucket->begin();
  SievingPrime* end = bucket->end();
  std::size_t wheelIndex = prime->getWheelIndex();

  for (; prime != end; prime++)
  {
    std::size_t sievingPrime = prime->getSievingPrime();
    std::size_t i = prime->getMultipleIndex();
    std::size_t dist0 = sievingPrime * 6 + 1;
    std::size_t dist1 = sievingPrime * 4 + 0;
    std::size_t dist2 = sievingPrime * 2 + 0;
    std::size_t dist6 = sievingPrime * 6 + 0;

    ASSERT(wheelIndex >= 56);
    ASSERT(wheelIndex <= 63);

    switch (wheelIndex)
    {
      default: UNREACHABLE;

      for (;;)
      {
        case 56: CHECK_FINISHED(56); sieve[i] &= BIT7; i += dist0; FALLTHROUGH;
        case 57: CHECK_FINISHED(57); sieve[i] &= BIT0; i += dist1; FALLTHROUGH;
        case 58: CHECK_FINISHED(58); sieve[i] &= BIT1; i += dist2; FALLTHROUGH;
        case 59: CHECK_FINISHED(59); sieve[i] &= BIT2; i += dist1; FALLTHROUGH;
        case 60: CHECK_FINISHED(60); sieve[i] &= BIT3; i += dist2; FALLTHROUGH;
        case 61: CHECK_FINISHED(61); sieve[i] &= BIT4; i += dist1; FALLTHROUGH;
        case 62: CHECK_FINISHED(62); sieve[i] &= BIT5; i += dist6; FALLTHROUGH;
        case 63: CHECK_FINISHED(63); sieve[i] &= BIT6; i += dist2;
      }
    }
  }
}

} // namespace
