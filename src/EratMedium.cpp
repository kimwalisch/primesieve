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
/// Copyright (C) 2020 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/EratMedium.hpp>
#include <primesieve/bits.hpp>
#include <primesieve/Bucket.hpp>
#include <primesieve/MemoryPool.hpp>
#include <primesieve/Wheel.hpp>
#include <primesieve/primesieve_error.hpp>
#include <primesieve/unlikely.hpp>

#include <stdint.h>
#include <cassert>

/// This macro sorts the current sieving prime by its
/// wheelIndex after sieving has finished. When we then
/// iterate over the sieving primes in the next segment the
/// 'switch (wheelIndex)' branch will be predicted
/// correctly by the CPU.
///
#define CHECK_FINISHED(wheelIndex) \
  if_unlikely(p >= sieveEnd) \
  { \
    multipleIndex = (uint64_t) (p - sieveEnd); \
    if (memoryPool_.isFullBucket(buckets_[wheelIndex])) \
      memoryPool_.addBucket(buckets_[wheelIndex]); \
    buckets_[wheelIndex]++->set(sievingPrime, multipleIndex, wheelIndex); \
    break; \
  }

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
  if (maxPrime > sieveSize * 9)
    throw primesieve_error("EratMedium: maxPrime > sieveSize * 9");

  enabled_ = true;
  maxPrime_ = maxPrime;
  buckets_.fill(nullptr);

  Wheel::init(stop, sieveSize);
}

/// Add a new sieving prime to EratMedium
void EratMedium::storeSievingPrime(uint64_t prime, uint64_t multipleIndex, uint64_t wheelIndex)
{
  assert(prime <= maxPrime_);
  uint64_t sievingPrime = prime / 30;

  if (memoryPool_.isFullBucket(buckets_[wheelIndex]))
    memoryPool_.addBucket(buckets_[wheelIndex]);

  buckets_[wheelIndex]++->set(sievingPrime, multipleIndex, wheelIndex);
}

void EratMedium::crossOff(uint8_t* sieve, uint64_t sieveSize)
{
  // Make a copy of buckets, then reset it
  auto buckets = buckets_;
  buckets_.fill(nullptr);
  uint8_t* sieveEnd = sieve + sieveSize;

  // Iterate over the 64 bucket lists.
  // The 1st list contains sieving primes with wheelIndex = 0.
  // The 2nd list contains sieving primes with wheelIndex = 1.
  // The 3rd list contains sieving primes with wheelIndex = 2.
  // ...
  for (uint64_t i = 0; i < 64; i++)
  {
    if (!buckets[i])
      continue;

    Bucket* bucket = memoryPool_.getBucket(buckets[i]);
    bucket->setEnd(buckets[i]);
    uint64_t wheel_index = i;

    // Iterate over the current bucket list.
    // For each bucket cross off the
    // multiples of its sieving primes.
    while (bucket)
    {
      switch (wheel_index / 8)
      {
        case 0: crossOff_7 (sieve, sieveEnd, bucket); break;
        case 1: crossOff_11(sieve, sieveEnd, bucket); break;
        case 2: crossOff_13(sieve, sieveEnd, bucket); break;
        case 3: crossOff_17(sieve, sieveEnd, bucket); break;
        case 4: crossOff_19(sieve, sieveEnd, bucket); break;
        case 5: crossOff_23(sieve, sieveEnd, bucket); break;
        case 6: crossOff_29(sieve, sieveEnd, bucket); break;
        case 7: crossOff_31(sieve, sieveEnd, bucket); break;
      }

      Bucket* processed = bucket;
      bucket = bucket->next();
      memoryPool_.freeBucket(processed);
    }
  }
}

/// For sieving primes of type n % 30 == 7
void EratMedium::crossOff_7(uint8_t* sieve, uint8_t* sieveEnd, Bucket* bucket)
{
  SievingPrime* prime = bucket->begin();
  SievingPrime* end = bucket->end();
  uint64_t wheelIndex = prime->getWheelIndex();

  for (; prime != end; prime++)
  {
    uint64_t sievingPrime = prime->getSievingPrime();
    uint64_t multipleIndex = prime->getMultipleIndex();
    uint8_t* p = sieve + multipleIndex;

    uint64_t dist0 = sievingPrime * 6 + 1;
    uint64_t dist1 = sievingPrime * 4 + 1;
    uint64_t dist2 = sievingPrime * 2 + 0;
    uint64_t dist3 = sievingPrime * 4 + 1;
    uint64_t dist4 = sievingPrime * 2 + 1;
    uint64_t dist5 = sievingPrime * 4 + 1;
    uint64_t dist6 = sievingPrime * 6 + 1;
    uint64_t dist7 = sievingPrime * 2 + 1;

    switch (wheelIndex)
    {
      for (;;)
      {
        case 0: CHECK_FINISHED(0);
                *p &= BIT0; p += dist0;
        case 1: CHECK_FINISHED(1);
                *p &= BIT4; p += dist1;
        case 2: CHECK_FINISHED(2);
                *p &= BIT3; p += dist2;
        case 3: CHECK_FINISHED(3);
                *p &= BIT7; p += dist3;
        case 4: CHECK_FINISHED(4);
                *p &= BIT6; p += dist4;
        case 5: CHECK_FINISHED(5);
                *p &= BIT2; p += dist5;
        case 6: CHECK_FINISHED(6);
                *p &= BIT1; p += dist6;
        case 7: CHECK_FINISHED(7);
                *p &= BIT5; p += dist7;
      }
    }
  }
}

/// For sieving primes of type n % 30 == 11
void EratMedium::crossOff_11(uint8_t* sieve, uint8_t* sieveEnd, Bucket* bucket)
{
  SievingPrime* prime = bucket->begin();
  SievingPrime* end = bucket->end();
  uint64_t wheelIndex = prime->getWheelIndex();

  for (; prime != end; prime++)
  {
    uint64_t sievingPrime = prime->getSievingPrime();
    uint64_t multipleIndex = prime->getMultipleIndex();
    uint8_t* p = sieve + multipleIndex;

    uint64_t dist0 = sievingPrime * 6 + 2;
    uint64_t dist1 = sievingPrime * 4 + 1;
    uint64_t dist2 = sievingPrime * 2 + 1;
    uint64_t dist3 = sievingPrime * 4 + 2;
    uint64_t dist4 = sievingPrime * 2 + 0;
    uint64_t dist5 = sievingPrime * 4 + 2;
    uint64_t dist6 = sievingPrime * 6 + 2;
    uint64_t dist7 = sievingPrime * 2 + 1;

    switch (wheelIndex)
    {
      for (;;)
      {
        case  8: CHECK_FINISHED(8);
                 *p &= BIT1; p += dist0;
        case  9: CHECK_FINISHED(9);
                 *p &= BIT3; p += dist1;
        case 10: CHECK_FINISHED(10);
                 *p &= BIT7; p += dist2;
        case 11: CHECK_FINISHED(11);
                 *p &= BIT5; p += dist3;
        case 12: CHECK_FINISHED(12);
                 *p &= BIT0; p += dist4;
        case 13: CHECK_FINISHED(13);
                 *p &= BIT6; p += dist5;
        case 14: CHECK_FINISHED(14);
                 *p &= BIT2; p += dist6;
        case 15: CHECK_FINISHED(15);
                 *p &= BIT4; p += dist7;
      }
    }
  }
}

/// For sieving primes of type n % 30 == 13
void EratMedium::crossOff_13(uint8_t* sieve, uint8_t* sieveEnd, Bucket* bucket)
{
  SievingPrime* prime = bucket->begin();
  SievingPrime* end = bucket->end();
  uint64_t wheelIndex = prime->getWheelIndex();

  for (; prime != end; prime++)
  {
    uint64_t sievingPrime = prime->getSievingPrime();
    uint64_t multipleIndex = prime->getMultipleIndex();
    uint8_t* p = sieve + multipleIndex;

    uint64_t dist0 = sievingPrime * 6 + 2;
    uint64_t dist1 = sievingPrime * 4 + 2;
    uint64_t dist2 = sievingPrime * 2 + 1;
    uint64_t dist3 = sievingPrime * 4 + 2;
    uint64_t dist4 = sievingPrime * 2 + 1;
    uint64_t dist5 = sievingPrime * 4 + 1;
    uint64_t dist6 = sievingPrime * 6 + 3;
    uint64_t dist7 = sievingPrime * 2 + 1;

    switch (wheelIndex)
    {
      for (;;)
      {
        case 16: CHECK_FINISHED(16);
                 *p &= BIT2; p += dist0;
        case 17: CHECK_FINISHED(17);
                 *p &= BIT7; p += dist1;
        case 18: CHECK_FINISHED(18);
                 *p &= BIT5; p += dist2;
        case 19: CHECK_FINISHED(19);
                 *p &= BIT4; p += dist3;
        case 20: CHECK_FINISHED(20);
                 *p &= BIT1; p += dist4;
        case 21: CHECK_FINISHED(21);
                 *p &= BIT0; p += dist5;
        case 22: CHECK_FINISHED(22);
                 *p &= BIT6; p += dist6;
        case 23: CHECK_FINISHED(23);
                 *p &= BIT3; p += dist7;
      }
    }
  }
}

/// For sieving primes of type n % 30 == 17
void EratMedium::crossOff_17(uint8_t* sieve, uint8_t* sieveEnd, Bucket* bucket)
{
  SievingPrime* prime = bucket->begin();
  SievingPrime* end = bucket->end();
  uint64_t wheelIndex = prime->getWheelIndex();

  for (; prime != end; prime++)
  {
    uint64_t sievingPrime = prime->getSievingPrime();
    uint64_t multipleIndex = prime->getMultipleIndex();
    uint8_t* p = sieve + multipleIndex;

    uint64_t dist0 = sievingPrime * 6 + 3;
    uint64_t dist1 = sievingPrime * 4 + 3;
    uint64_t dist2 = sievingPrime * 2 + 1;
    uint64_t dist3 = sievingPrime * 4 + 2;
    uint64_t dist4 = sievingPrime * 2 + 1;
    uint64_t dist5 = sievingPrime * 4 + 2;
    uint64_t dist6 = sievingPrime * 6 + 4;
    uint64_t dist7 = sievingPrime * 2 + 1;

    switch (wheelIndex)
    {
      for (;;)
      {
        case 24: CHECK_FINISHED(24);
                 *p &= BIT3; p += dist0;
        case 25: CHECK_FINISHED(25);
                 *p &= BIT6; p += dist1;
        case 26: CHECK_FINISHED(26);
                 *p &= BIT0; p += dist2;
        case 27: CHECK_FINISHED(27);
                 *p &= BIT1; p += dist3;
        case 28: CHECK_FINISHED(28);
                 *p &= BIT4; p += dist4;
        case 29: CHECK_FINISHED(29);
                 *p &= BIT5; p += dist5;
        case 30: CHECK_FINISHED(30);
                 *p &= BIT7; p += dist6;
        case 31: CHECK_FINISHED(31);
                 *p &= BIT2; p += dist7;
      }
    }
  }
}

/// For sieving primes of type n % 30 == 19
void EratMedium::crossOff_19(uint8_t* sieve, uint8_t* sieveEnd, Bucket* bucket)
{
  SievingPrime* prime = bucket->begin();
  SievingPrime* end = bucket->end();
  uint64_t wheelIndex = prime->getWheelIndex();

  for (; prime != end; prime++)
  {
    uint64_t sievingPrime = prime->getSievingPrime();
    uint64_t multipleIndex = prime->getMultipleIndex();
    uint8_t* p = sieve + multipleIndex;

    uint64_t dist0 = sievingPrime * 6 + 4;
    uint64_t dist1 = sievingPrime * 4 + 2;
    uint64_t dist2 = sievingPrime * 2 + 2;
    uint64_t dist3 = sievingPrime * 4 + 2;
    uint64_t dist4 = sievingPrime * 2 + 1;
    uint64_t dist5 = sievingPrime * 4 + 3;
    uint64_t dist6 = sievingPrime * 6 + 4;
    uint64_t dist7 = sievingPrime * 2 + 1;

    switch (wheelIndex)
    {
      for (;;)
      {
        case 32: CHECK_FINISHED(32);
                 *p &= BIT4; p += dist0;
        case 33: CHECK_FINISHED(33);
                 *p &= BIT2; p += dist1;
        case 34: CHECK_FINISHED(34);
                 *p &= BIT6; p += dist2;
        case 35: CHECK_FINISHED(35);
                 *p &= BIT0; p += dist3;
        case 36: CHECK_FINISHED(36);
                 *p &= BIT5; p += dist4;
        case 37: CHECK_FINISHED(37);
                 *p &= BIT7; p += dist5;
        case 38: CHECK_FINISHED(38);
                 *p &= BIT3; p += dist6;
        case 39: CHECK_FINISHED(39);
                 *p &= BIT1; p += dist7;
      }
    }
  }
}

/// For sieving primes of type n % 30 == 23
void EratMedium::crossOff_23(uint8_t* sieve, uint8_t* sieveEnd, Bucket* bucket)
{
  SievingPrime* prime = bucket->begin();
  SievingPrime* end = bucket->end();
  uint64_t wheelIndex = prime->getWheelIndex();

  for (; prime != end; prime++)
  {
    uint64_t sievingPrime = prime->getSievingPrime();
    uint64_t multipleIndex = prime->getMultipleIndex();
    uint8_t* p = sieve + multipleIndex;

    uint64_t dist0 = sievingPrime * 6 + 5;
    uint64_t dist1 = sievingPrime * 4 + 3;
    uint64_t dist2 = sievingPrime * 2 + 1;
    uint64_t dist3 = sievingPrime * 4 + 3;
    uint64_t dist4 = sievingPrime * 2 + 2;
    uint64_t dist5 = sievingPrime * 4 + 3;
    uint64_t dist6 = sievingPrime * 6 + 5;
    uint64_t dist7 = sievingPrime * 2 + 1;

    switch (wheelIndex)
    {
      for (;;)
      {
        case 40: CHECK_FINISHED(40);
                 *p &= BIT5; p += dist0;
        case 41: CHECK_FINISHED(41);
                 *p &= BIT1; p += dist1;
        case 42: CHECK_FINISHED(42);
                 *p &= BIT2; p += dist2;
        case 43: CHECK_FINISHED(43);
                 *p &= BIT6; p += dist3;
        case 44: CHECK_FINISHED(44);
                 *p &= BIT7; p += dist4;
        case 45: CHECK_FINISHED(45);
                 *p &= BIT3; p += dist5;
        case 46: CHECK_FINISHED(46);
                 *p &= BIT4; p += dist6;
        case 47: CHECK_FINISHED(47);
                 *p &= BIT0; p += dist7;
      }
    }
  }
}

/// For sieving primes of type n % 30 == 29
void EratMedium::crossOff_29(uint8_t* sieve, uint8_t* sieveEnd, Bucket* bucket)
{
  SievingPrime* prime = bucket->begin();
  SievingPrime* end = bucket->end();
  uint64_t wheelIndex = prime->getWheelIndex();

  for (; prime != end; prime++)
  {
    uint64_t sievingPrime = prime->getSievingPrime();
    uint64_t multipleIndex = prime->getMultipleIndex();
    uint8_t* p = sieve + multipleIndex;

    uint64_t dist0 = sievingPrime * 6 + 6;
    uint64_t dist1 = sievingPrime * 4 + 4;
    uint64_t dist2 = sievingPrime * 2 + 2;
    uint64_t dist3 = sievingPrime * 4 + 4;
    uint64_t dist4 = sievingPrime * 2 + 2;
    uint64_t dist5 = sievingPrime * 4 + 4;
    uint64_t dist6 = sievingPrime * 6 + 5;
    uint64_t dist7 = sievingPrime * 2 + 2;

    switch (wheelIndex)
    {
      for (;;)
      {
        case 48: CHECK_FINISHED(48);
                 *p &= BIT6; p += dist0;
        case 49: CHECK_FINISHED(49);
                 *p &= BIT5; p += dist1;
        case 50: CHECK_FINISHED(50);
                 *p &= BIT4; p += dist2;
        case 51: CHECK_FINISHED(51);
                 *p &= BIT3; p += dist3;
        case 52: CHECK_FINISHED(52);
                 *p &= BIT2; p += dist4;
        case 53: CHECK_FINISHED(53);
                 *p &= BIT1; p += dist5;
        case 54: CHECK_FINISHED(54);
                 *p &= BIT0; p += dist6;
        case 55: CHECK_FINISHED(55);
                 *p &= BIT7; p += dist7;
      }
    }
  }
}

/// For sieving primes of type n % 30 == 1
void EratMedium::crossOff_31(uint8_t* sieve, uint8_t* sieveEnd, Bucket* bucket)
{
  SievingPrime* prime = bucket->begin();
  SievingPrime* end = bucket->end();
  uint64_t wheelIndex = prime->getWheelIndex();

  for (; prime != end; prime++)
  {
    uint64_t sievingPrime = prime->getSievingPrime();
    uint64_t multipleIndex = prime->getMultipleIndex();
    uint8_t* p = sieve + multipleIndex;

    uint64_t dist0 = sievingPrime * 6 + 1;
    uint64_t dist1 = sievingPrime * 4 + 0;
    uint64_t dist2 = sievingPrime * 2 + 0;
    uint64_t dist3 = sievingPrime * 4 + 0;
    uint64_t dist4 = sievingPrime * 2 + 0;
    uint64_t dist5 = sievingPrime * 4 + 0;
    uint64_t dist6 = sievingPrime * 6 + 0;
    uint64_t dist7 = sievingPrime * 2 + 0;

    switch (wheelIndex)
    {
      for (;;)
      {
        case 56: CHECK_FINISHED(56);
                 *p &= BIT7; p += dist0;
        case 57: CHECK_FINISHED(57);
                 *p &= BIT0; p += dist1;
        case 58: CHECK_FINISHED(58);
                 *p &= BIT1; p += dist2;
        case 59: CHECK_FINISHED(59);
                 *p &= BIT2; p += dist3;
        case 60: CHECK_FINISHED(60);
                 *p &= BIT3; p += dist4;
        case 61: CHECK_FINISHED(61);
                 *p &= BIT4; p += dist5;
        case 62: CHECK_FINISHED(62);
                 *p &= BIT5; p += dist6;
        case 63: CHECK_FINISHED(63);
                 *p &= BIT6; p += dist7;
      }
    }
  }
}

} // namespace
