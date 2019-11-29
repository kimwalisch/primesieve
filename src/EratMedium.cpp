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
/// Copyright (C) 2019 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/EratMedium.hpp>
#include <primesieve/Bucket.hpp>
#include <primesieve/MemoryPool.hpp>
#include <primesieve/Wheel.hpp>
#include <primesieve/primesieve_error.hpp>
#include <primesieve/bits.hpp>

#include <stdint.h>
#include <cassert>

/// This macro sorts the current sieving prime by its
/// wheelIndex after sieving has finished. When we then
/// iterate over the sieving primes in the next segment the
/// 'switch (wheelIndex)' branch will be predicted
/// correctly by the CPU.
///
#define SORT_SIEVING_PRIME(wheelIndex) \
  sort ## wheelIndex: \
  multipleIndex = (uint64_t) (p - sieveEnd); \
  if (memoryPool_.isFullBucket(sievingPrimes_[wheelIndex])) \
    memoryPool_.addBucket(sievingPrimes_[wheelIndex]); \
  sievingPrimes_[wheelIndex]++->set(sievingPrime, multipleIndex, wheelIndex); \
  continue;

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
  sievingPrimes_.fill(nullptr);

  Wheel::init(stop, sieveSize);
}

/// Add a new sieving prime to EratMedium
void EratMedium::storeSievingPrime(uint64_t prime, uint64_t multipleIndex, uint64_t wheelIndex)
{
  assert(prime <= maxPrime_);
  uint64_t sievingPrime = prime / 30;

  if (memoryPool_.isFullBucket(sievingPrimes_[wheelIndex]))
    memoryPool_.addBucket(sievingPrimes_[wheelIndex]);

  sievingPrimes_[wheelIndex]++->set(sievingPrime, multipleIndex, wheelIndex);
}

void EratMedium::crossOff(uint8_t* sieve, uint64_t sieveSize)
{
  // Make a copy of sievingPrimes, then reset it
  auto lists = sievingPrimes_;
  sievingPrimes_.fill(nullptr);
  uint8_t* sieveEnd = sieve + sieveSize;

  // Iterate over the 64 bucket lists.
  // The 1st list contains sieving primes with wheelIndex = 0.
  // The 2nd list contains sieving primes with wheelIndex = 1.
  // The 3rd list contains sieving primes with wheelIndex = 2.
  // ...
  for (SievingPrime* sievingPrime : lists)
  {
    if (!sievingPrime)
      continue;

    Bucket* bucket = memoryPool_.getBucket(sievingPrime);
    bucket->setEnd(sievingPrime);

    // Iterate over the current bucket list.
    // For each bucket cross off the
    // multiples of its sieving primes.
    while (bucket)
    {
      crossOff(sieve, sieveEnd, bucket);
      Bucket* processed = bucket;
      bucket = bucket->next();
      memoryPool_.freeBucket(processed);
    }
  }
}

void EratMedium::crossOff(uint8_t* sieve, uint8_t* sieveEnd, Bucket* bucket)
{
  SievingPrime* prime = bucket->begin();
  uint64_t primeType = prime->getWheelIndex() / 8;

  switch (primeType)
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
        case 0: if (p >= sieveEnd) goto sort0;
                *p &= BIT0; p += dist0;
        case 1: if (p >= sieveEnd) goto sort1;
                *p &= BIT4; p += dist1;
        case 2: if (p >= sieveEnd) goto sort2;
                *p &= BIT3; p += dist2;
        case 3: if (p >= sieveEnd) goto sort3;
                *p &= BIT7; p += dist3;
        case 4: if (p >= sieveEnd) goto sort4;
                *p &= BIT6; p += dist4;
        case 5: if (p >= sieveEnd) goto sort5;
                *p &= BIT2; p += dist5;
        case 6: if (p >= sieveEnd) goto sort6;
                *p &= BIT1; p += dist6;
        case 7: if (p >= sieveEnd) goto sort7;
                *p &= BIT5; p += dist7;
      }

      SORT_SIEVING_PRIME(0)
      SORT_SIEVING_PRIME(1)
      SORT_SIEVING_PRIME(2)
      SORT_SIEVING_PRIME(3)
      SORT_SIEVING_PRIME(4)
      SORT_SIEVING_PRIME(5)
      SORT_SIEVING_PRIME(6)
      SORT_SIEVING_PRIME(7)
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
        case  8: if (p >= sieveEnd) goto sort8;
                 *p &= BIT1; p += dist0;
        case  9: if (p >= sieveEnd) goto sort9;
                 *p &= BIT3; p += dist1;
        case 10: if (p >= sieveEnd) goto sort10;
                 *p &= BIT7; p += dist2;
        case 11: if (p >= sieveEnd) goto sort11;
                 *p &= BIT5; p += dist3;
        case 12: if (p >= sieveEnd) goto sort12;
                 *p &= BIT0; p += dist4;
        case 13: if (p >= sieveEnd) goto sort13;
                 *p &= BIT6; p += dist5;
        case 14: if (p >= sieveEnd) goto sort14;
                 *p &= BIT2; p += dist6;
        case 15: if (p >= sieveEnd) goto sort15;
                 *p &= BIT4; p += dist7;
      }

      SORT_SIEVING_PRIME(8)
      SORT_SIEVING_PRIME(9)
      SORT_SIEVING_PRIME(10)
      SORT_SIEVING_PRIME(11)
      SORT_SIEVING_PRIME(12)
      SORT_SIEVING_PRIME(13)
      SORT_SIEVING_PRIME(14)
      SORT_SIEVING_PRIME(15)
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
        case 16: if (p >= sieveEnd) goto sort16;
                 *p &= BIT2; p += dist0;
        case 17: if (p >= sieveEnd) goto sort17;
                 *p &= BIT7; p += dist1;
        case 18: if (p >= sieveEnd) goto sort18;
                 *p &= BIT5; p += dist2;
        case 19: if (p >= sieveEnd) goto sort19;
                 *p &= BIT4; p += dist3;
        case 20: if (p >= sieveEnd) goto sort20;
                 *p &= BIT1; p += dist4;
        case 21: if (p >= sieveEnd) goto sort21;
                 *p &= BIT0; p += dist5;
        case 22: if (p >= sieveEnd) goto sort22;
                 *p &= BIT6; p += dist6;
        case 23: if (p >= sieveEnd) goto sort23;
                 *p &= BIT3; p += dist7;
      }

      SORT_SIEVING_PRIME(16)
      SORT_SIEVING_PRIME(17)
      SORT_SIEVING_PRIME(18)
      SORT_SIEVING_PRIME(19)
      SORT_SIEVING_PRIME(20)
      SORT_SIEVING_PRIME(21)
      SORT_SIEVING_PRIME(22)
      SORT_SIEVING_PRIME(23)
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
        case 24: if (p >= sieveEnd) goto sort24;
                 *p &= BIT3; p += dist0;
        case 25: if (p >= sieveEnd) goto sort25;
                 *p &= BIT6; p += dist1;
        case 26: if (p >= sieveEnd) goto sort26;
                 *p &= BIT0; p += dist2;
        case 27: if (p >= sieveEnd) goto sort27;
                 *p &= BIT1; p += dist3;
        case 28: if (p >= sieveEnd) goto sort28;
                 *p &= BIT4; p += dist4;
        case 29: if (p >= sieveEnd) goto sort29;
                 *p &= BIT5; p += dist5;
        case 30: if (p >= sieveEnd) goto sort30;
                 *p &= BIT7; p += dist6;
        case 31: if (p >= sieveEnd) goto sort31;
                 *p &= BIT2; p += dist7;
      }

      SORT_SIEVING_PRIME(24)
      SORT_SIEVING_PRIME(25)
      SORT_SIEVING_PRIME(26)
      SORT_SIEVING_PRIME(27)
      SORT_SIEVING_PRIME(28)
      SORT_SIEVING_PRIME(29)
      SORT_SIEVING_PRIME(30)
      SORT_SIEVING_PRIME(31)
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
        case 32: if (p >= sieveEnd) goto sort32;
                 *p &= BIT4; p += dist0;
        case 33: if (p >= sieveEnd) goto sort33;
                 *p &= BIT2; p += dist1;
        case 34: if (p >= sieveEnd) goto sort34;
                 *p &= BIT6; p += dist2;
        case 35: if (p >= sieveEnd) goto sort35;
                 *p &= BIT0; p += dist3;
        case 36: if (p >= sieveEnd) goto sort36;
                 *p &= BIT5; p += dist4;
        case 37: if (p >= sieveEnd) goto sort37;
                 *p &= BIT7; p += dist5;
        case 38: if (p >= sieveEnd) goto sort38;
                 *p &= BIT3; p += dist6;
        case 39: if (p >= sieveEnd) goto sort39;
                 *p &= BIT1; p += dist7;
      }

      SORT_SIEVING_PRIME(32)
      SORT_SIEVING_PRIME(33)
      SORT_SIEVING_PRIME(34)
      SORT_SIEVING_PRIME(35)
      SORT_SIEVING_PRIME(36)
      SORT_SIEVING_PRIME(37)
      SORT_SIEVING_PRIME(38)
      SORT_SIEVING_PRIME(39)
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
        case 40: if (p >= sieveEnd) goto sort40;
                 *p &= BIT5; p += dist0;
        case 41: if (p >= sieveEnd) goto sort41;
                 *p &= BIT1; p += dist1;
        case 42: if (p >= sieveEnd) goto sort42;
                 *p &= BIT2; p += dist2;
        case 43: if (p >= sieveEnd) goto sort43;
                 *p &= BIT6; p += dist3;
        case 44: if (p >= sieveEnd) goto sort44;
                 *p &= BIT7; p += dist4;
        case 45: if (p >= sieveEnd) goto sort45;
                 *p &= BIT3; p += dist5;
        case 46: if (p >= sieveEnd) goto sort46;
                 *p &= BIT4; p += dist6;
        case 47: if (p >= sieveEnd) goto sort47;
                 *p &= BIT0; p += dist7;
      }

      SORT_SIEVING_PRIME(40)
      SORT_SIEVING_PRIME(41)
      SORT_SIEVING_PRIME(42)
      SORT_SIEVING_PRIME(43)
      SORT_SIEVING_PRIME(44)
      SORT_SIEVING_PRIME(45)
      SORT_SIEVING_PRIME(46)
      SORT_SIEVING_PRIME(47)
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
        case 48: if (p >= sieveEnd) goto sort48;
                 *p &= BIT6; p += dist0;
        case 49: if (p >= sieveEnd) goto sort49;
                 *p &= BIT5; p += dist1;
        case 50: if (p >= sieveEnd) goto sort50;
                 *p &= BIT4; p += dist2;
        case 51: if (p >= sieveEnd) goto sort51;
                 *p &= BIT3; p += dist3;
        case 52: if (p >= sieveEnd) goto sort52;
                 *p &= BIT2; p += dist4;
        case 53: if (p >= sieveEnd) goto sort53;
                 *p &= BIT1; p += dist5;
        case 54: if (p >= sieveEnd) goto sort54;
                 *p &= BIT0; p += dist6;
        case 55: if (p >= sieveEnd) goto sort55;
                 *p &= BIT7; p += dist7;
      }

      SORT_SIEVING_PRIME(48)
      SORT_SIEVING_PRIME(49)
      SORT_SIEVING_PRIME(50)
      SORT_SIEVING_PRIME(51)
      SORT_SIEVING_PRIME(52)
      SORT_SIEVING_PRIME(53)
      SORT_SIEVING_PRIME(54)
      SORT_SIEVING_PRIME(55)
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
        case 56: if (p >= sieveEnd) goto sort56;
                 *p &= BIT7; p += dist0;
        case 57: if (p >= sieveEnd) goto sort57;
                 *p &= BIT0; p += dist1;
        case 58: if (p >= sieveEnd) goto sort58;
                 *p &= BIT1; p += dist2;
        case 59: if (p >= sieveEnd) goto sort59;
                 *p &= BIT2; p += dist3;
        case 60: if (p >= sieveEnd) goto sort60;
                 *p &= BIT3; p += dist4;
        case 61: if (p >= sieveEnd) goto sort61;
                 *p &= BIT4; p += dist5;
        case 62: if (p >= sieveEnd) goto sort62;
                 *p &= BIT5; p += dist6;
        case 63: if (p >= sieveEnd) goto sort63;
                 *p &= BIT6; p += dist7;
      }

      SORT_SIEVING_PRIME(56)
      SORT_SIEVING_PRIME(57)
      SORT_SIEVING_PRIME(58)
      SORT_SIEVING_PRIME(59)
      SORT_SIEVING_PRIME(60)
      SORT_SIEVING_PRIME(61)
      SORT_SIEVING_PRIME(62)
      SORT_SIEVING_PRIME(63)
    }
  }
}

} // namespace
