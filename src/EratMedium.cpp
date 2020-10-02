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
#include <primesieve/macros.hpp>
#include <primesieve/MemoryPool.hpp>

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
    if (Bucket::isFull(buckets_[wheelIndex])) \
      memoryPool_.addBucket(buckets_[wheelIndex]); \
    buckets_[wheelIndex]++->set(sievingPrime, multipleIndex, wheelIndex); \
    break; \
  }

namespace primesieve {

/// @stop:      Upper bound for sieving
/// @sieveSize: Sieve size in bytes
/// @maxPrime:  Sieving primes <= maxPrime
///
void EratMedium::init(uint64_t stop,
                      uint64_t MAYBE_UNUSED(sieveSize),
                      uint64_t maxPrime)
{
  assert(maxPrime <= sieveSize * 9);
  assert(sieveSize * 2 <= SievingPrime::MAX_MULTIPLEINDEX + 1);

  enabled_ = true;
  stop_ = stop;
  maxPrime_ = maxPrime;
  buckets_.fill(nullptr);
}

/// Add a new sieving prime to EratMedium
void EratMedium::storeSievingPrime(uint64_t prime, uint64_t multipleIndex, uint64_t wheelIndex)
{
  assert(prime <= maxPrime_);
  uint64_t sievingPrime = prime / 30;

  if (Bucket::isFull(buckets_[wheelIndex]))
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

    Bucket* bucket = Bucket::get(buckets[i]);
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
    uint64_t dist4 = sievingPrime * 2 + 1;

    // By using switch(n & 7) we let the compiler know that
    // n is always within [0, 7]. This trick reduces the
    // number of instructions as the compiler now does not
    // add a check if (n > 7) which skips to the end of the
    // switch statement.
    switch (wheelIndex & 7)
    {
      for (;;)
      {
        case 0: CHECK_FINISHED(0); *p &= BIT0; p += dist0; FALLTHROUGH
        case 1: CHECK_FINISHED(1); *p &= BIT4; p += dist1; FALLTHROUGH
        case 2: CHECK_FINISHED(2); *p &= BIT3; p += dist2; FALLTHROUGH
        case 3: CHECK_FINISHED(3); *p &= BIT7; p += dist1; FALLTHROUGH
        case 4: CHECK_FINISHED(4); *p &= BIT6; p += dist4; FALLTHROUGH
        case 5: CHECK_FINISHED(5); *p &= BIT2; p += dist1; FALLTHROUGH
        case 6: CHECK_FINISHED(6); *p &= BIT1; p += dist0; FALLTHROUGH
        case 7: CHECK_FINISHED(7); *p &= BIT5; p += dist4;
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

    switch (wheelIndex & 7)
    {
      for (;;)
      {
        case 0: CHECK_FINISHED( 8); *p &= BIT1; p += dist0; FALLTHROUGH
        case 1: CHECK_FINISHED( 9); *p &= BIT3; p += dist1; FALLTHROUGH
        case 2: CHECK_FINISHED(10); *p &= BIT7; p += dist2; FALLTHROUGH
        case 3: CHECK_FINISHED(11); *p &= BIT5; p += dist3; FALLTHROUGH
        case 4: CHECK_FINISHED(12); *p &= BIT0; p += dist4; FALLTHROUGH
        case 5: CHECK_FINISHED(13); *p &= BIT6; p += dist3; FALLTHROUGH
        case 6: CHECK_FINISHED(14); *p &= BIT2; p += dist0; FALLTHROUGH
        case 7: CHECK_FINISHED(15); *p &= BIT4; p += dist2;
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
    uint64_t dist5 = sievingPrime * 4 + 1;
    uint64_t dist6 = sievingPrime * 6 + 3;

    switch (wheelIndex & 7)
    {
      for (;;)
      {
        case 0: CHECK_FINISHED(16); *p &= BIT2; p += dist0; FALLTHROUGH
        case 1: CHECK_FINISHED(17); *p &= BIT7; p += dist1; FALLTHROUGH
        case 2: CHECK_FINISHED(18); *p &= BIT5; p += dist2; FALLTHROUGH
        case 3: CHECK_FINISHED(19); *p &= BIT4; p += dist1; FALLTHROUGH
        case 4: CHECK_FINISHED(20); *p &= BIT1; p += dist2; FALLTHROUGH
        case 5: CHECK_FINISHED(21); *p &= BIT0; p += dist5; FALLTHROUGH
        case 6: CHECK_FINISHED(22); *p &= BIT6; p += dist6; FALLTHROUGH
        case 7: CHECK_FINISHED(23); *p &= BIT3; p += dist2;
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
    uint64_t dist6 = sievingPrime * 6 + 4;

    switch (wheelIndex & 7)
    {
      for (;;)
      {
        case 0: CHECK_FINISHED(24); *p &= BIT3; p += dist0; FALLTHROUGH
        case 1: CHECK_FINISHED(25); *p &= BIT6; p += dist1; FALLTHROUGH
        case 2: CHECK_FINISHED(26); *p &= BIT0; p += dist2; FALLTHROUGH
        case 3: CHECK_FINISHED(27); *p &= BIT1; p += dist3; FALLTHROUGH
        case 4: CHECK_FINISHED(28); *p &= BIT4; p += dist2; FALLTHROUGH
        case 5: CHECK_FINISHED(29); *p &= BIT5; p += dist3; FALLTHROUGH
        case 6: CHECK_FINISHED(30); *p &= BIT7; p += dist6; FALLTHROUGH
        case 7: CHECK_FINISHED(31); *p &= BIT2; p += dist2;
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
    uint64_t dist4 = sievingPrime * 2 + 1;
    uint64_t dist5 = sievingPrime * 4 + 3;

    switch (wheelIndex & 7)
    {
      for (;;)
      {
        case 0: CHECK_FINISHED(32); *p &= BIT4; p += dist0; FALLTHROUGH
        case 1: CHECK_FINISHED(33); *p &= BIT2; p += dist1; FALLTHROUGH
        case 2: CHECK_FINISHED(34); *p &= BIT6; p += dist2; FALLTHROUGH
        case 3: CHECK_FINISHED(35); *p &= BIT0; p += dist1; FALLTHROUGH
        case 4: CHECK_FINISHED(36); *p &= BIT5; p += dist4; FALLTHROUGH
        case 5: CHECK_FINISHED(37); *p &= BIT7; p += dist5; FALLTHROUGH
        case 6: CHECK_FINISHED(38); *p &= BIT3; p += dist0; FALLTHROUGH
        case 7: CHECK_FINISHED(39); *p &= BIT1; p += dist4;
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
    uint64_t dist4 = sievingPrime * 2 + 2;

    switch (wheelIndex & 7)
    {
      for (;;)
      {
        case 0: CHECK_FINISHED(40); *p &= BIT5; p += dist0; FALLTHROUGH
        case 1: CHECK_FINISHED(41); *p &= BIT1; p += dist1; FALLTHROUGH
        case 2: CHECK_FINISHED(42); *p &= BIT2; p += dist2; FALLTHROUGH
        case 3: CHECK_FINISHED(43); *p &= BIT6; p += dist1; FALLTHROUGH
        case 4: CHECK_FINISHED(44); *p &= BIT7; p += dist4; FALLTHROUGH
        case 5: CHECK_FINISHED(45); *p &= BIT3; p += dist1; FALLTHROUGH
        case 6: CHECK_FINISHED(46); *p &= BIT4; p += dist0; FALLTHROUGH
        case 7: CHECK_FINISHED(47); *p &= BIT0; p += dist2;
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
    uint64_t dist6 = sievingPrime * 6 + 5;

    switch (wheelIndex & 7)
    {
      for (;;)
      {
        case 0: CHECK_FINISHED(48); *p &= BIT6; p += dist0; FALLTHROUGH
        case 1: CHECK_FINISHED(49); *p &= BIT5; p += dist1; FALLTHROUGH
        case 2: CHECK_FINISHED(50); *p &= BIT4; p += dist2; FALLTHROUGH
        case 3: CHECK_FINISHED(51); *p &= BIT3; p += dist1; FALLTHROUGH
        case 4: CHECK_FINISHED(52); *p &= BIT2; p += dist2; FALLTHROUGH
        case 5: CHECK_FINISHED(53); *p &= BIT1; p += dist1; FALLTHROUGH
        case 6: CHECK_FINISHED(54); *p &= BIT0; p += dist6; FALLTHROUGH
        case 7: CHECK_FINISHED(55); *p &= BIT7; p += dist2;
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
    uint64_t dist6 = sievingPrime * 6 + 0;

    switch (wheelIndex & 7)
    {
      for (;;)
      {
        case 0: CHECK_FINISHED(56); *p &= BIT7; p += dist0; FALLTHROUGH
        case 1: CHECK_FINISHED(57); *p &= BIT0; p += dist1; FALLTHROUGH
        case 2: CHECK_FINISHED(58); *p &= BIT1; p += dist2; FALLTHROUGH
        case 3: CHECK_FINISHED(59); *p &= BIT2; p += dist1; FALLTHROUGH
        case 4: CHECK_FINISHED(60); *p &= BIT3; p += dist2; FALLTHROUGH
        case 5: CHECK_FINISHED(61); *p &= BIT4; p += dist1; FALLTHROUGH
        case 6: CHECK_FINISHED(62); *p &= BIT5; p += dist6; FALLTHROUGH
        case 7: CHECK_FINISHED(63); *p &= BIT6; p += dist2;
      }
    }
  }
}

} // namespace
