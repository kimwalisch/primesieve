///
/// @file   EratMedium.cpp
/// @brief  Segmented sieve of Eratosthenes optimized for medium
///         sieving primes. EratMedium is similar to EratSmall except
///         that in EratMedium each sieving prime is sorted (by its
///         wheelIndex) after the sieving step. When we then iterate
///         over the sorted sieving primes in the next segment the
///         initial indirect branch i.e. switch(wheelIndex) is
///         predicted correctly by the CPU which improves performance
///         by up to 15%.
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/EratMedium.hpp>
#include <primesieve/Bucket.hpp>
#include <primesieve/MemoryPool.hpp>
#include <primesieve/Wheel.hpp>
#include <primesieve/primesieve_error.hpp>
#include <primesieve/types.hpp>
#include <primesieve/bits.hpp>

#include <stdint.h>
#include <cassert>

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
  memoryPool_.setAllocCount(lists_.size() * 2);

  Wheel::init(stop, sieveSize);
  resetLists();
}

/// Add a new sieving prime to EratMedium
void EratMedium::storeSievingPrime(uint64_t prime, uint64_t multipleIndex, uint64_t wheelIndex)
{
  assert(prime <= maxPrime_);
  uint64_t sievingPrime = prime / 30;
  if (!lists_[wheelIndex]->store(sievingPrime, multipleIndex, wheelIndex))
    memoryPool_.addBucket(lists_[wheelIndex]);
}

void EratMedium::resetLists()
{
  lists_.fill(nullptr);
  for (Bucket*& list : lists_)
    memoryPool_.addBucket(list);
}

/// Iterate over the 64 bucket lists (which
/// contain the sieving primes) and call
/// crossOff() for each bucket.
///
void EratMedium::crossOff(byte_t* sieve, uint64_t sieveSize)
{
  decltype(lists_) copyLists;
  copyLists = lists_;
  resetLists();

  for (Bucket* bucket : copyLists)
  {
    while (bucket)
    {
      crossOff(sieve, sieveSize, bucket->begin(), bucket->end());
      Bucket* processed = bucket;
      bucket = bucket->next();
      memoryPool_.freeBucket(processed);
    }
  }
}

/// Segmented sieve of Eratosthenes with wheel factorization
/// optimized for medium sieving primes that have a few
/// multiples per segment. This algorithm uses a hardcoded
/// modulo 30 wheel that skips multiples of 2, 3 and 5.
///
void EratMedium::crossOff(byte_t* sieve, uint64_t sieveSize, SievingPrime* prime, SievingPrime* end)
{
  for (; prime != end; prime++)
  {
    uint64_t sievingPrime = prime->getSievingPrime();
    uint64_t multipleIndex = prime->getMultipleIndex();
    uint64_t wheelIndex = prime->getWheelIndex();

    // This macro sorts the current sieving prime by its
    // wheelIndex after sieving has finished. When we
    // then iterate over the sieving primes in the next
    // segment the 'switch (wheelIndex)' branch will be
    // predicted correctly by the CPU.
    #define SORT_SIEVING_PRIME(wheelIndex) \
      out ## wheelIndex: \
      multipleIndex -= sieveSize; \
      if (!lists_[wheelIndex]->store(sievingPrime, multipleIndex, wheelIndex)) \
        memoryPool_.addBucket(lists_[wheelIndex]); \
      continue;

    switch (wheelIndex)
    {
      for (;;) // i*30 + 7
      {
        case 0: if (multipleIndex >= sieveSize) goto out0;
                sieve[multipleIndex] &= BIT0;
                multipleIndex += sievingPrime * 6 + 1;
        case 1: if (multipleIndex >= sieveSize) goto out1;
                sieve[multipleIndex] &= BIT4;
                multipleIndex += sievingPrime * 4 + 1;
        case 2: if (multipleIndex >= sieveSize) goto out2;
                sieve[multipleIndex] &= BIT3;
                multipleIndex += sievingPrime * 2 + 0;
        case 3: if (multipleIndex >= sieveSize) goto out3;
                sieve[multipleIndex] &= BIT7;
                multipleIndex += sievingPrime * 4 + 1;
        case 4: if (multipleIndex >= sieveSize) goto out4;
                sieve[multipleIndex] &= BIT6;
                multipleIndex += sievingPrime * 2 + 1;
        case 5: if (multipleIndex >= sieveSize) goto out5;
                sieve[multipleIndex] &= BIT2;
                multipleIndex += sievingPrime * 4 + 1;
        case 6: if (multipleIndex >= sieveSize) goto out6;
                sieve[multipleIndex] &= BIT1;
                multipleIndex += sievingPrime * 6 + 1;
        case 7: if (multipleIndex >= sieveSize) goto out7;
                sieve[multipleIndex] &= BIT5;
                multipleIndex += sievingPrime * 2 + 1;
      }
      SORT_SIEVING_PRIME(0)
      SORT_SIEVING_PRIME(1)
      SORT_SIEVING_PRIME(2)
      SORT_SIEVING_PRIME(3)
      SORT_SIEVING_PRIME(4)
      SORT_SIEVING_PRIME(5)
      SORT_SIEVING_PRIME(6)
      SORT_SIEVING_PRIME(7)

      for (;;) // i*30 + 11
      {
        case  8: if (multipleIndex >= sieveSize) goto out8;
                 sieve[multipleIndex] &= BIT1;
                 multipleIndex += sievingPrime * 6 + 2;
        case  9: if (multipleIndex >= sieveSize) goto out9;
                 sieve[multipleIndex] &= BIT3;
                 multipleIndex += sievingPrime * 4 + 1;
        case 10: if (multipleIndex >= sieveSize) goto out10;
                 sieve[multipleIndex] &= BIT7;
                 multipleIndex += sievingPrime * 2 + 1;
        case 11: if (multipleIndex >= sieveSize) goto out11;
                 sieve[multipleIndex] &= BIT5;
                 multipleIndex += sievingPrime * 4 + 2;
        case 12: if (multipleIndex >= sieveSize) goto out12;
                 sieve[multipleIndex] &= BIT0;
                 multipleIndex += sievingPrime * 2 + 0;
        case 13: if (multipleIndex >= sieveSize) goto out13;
                 sieve[multipleIndex] &= BIT6;
                 multipleIndex += sievingPrime * 4 + 2;
        case 14: if (multipleIndex >= sieveSize) goto out14;
                 sieve[multipleIndex] &= BIT2;
                 multipleIndex += sievingPrime * 6 + 2;
        case 15: if (multipleIndex >= sieveSize) goto out15;
                 sieve[multipleIndex] &= BIT4;
                 multipleIndex += sievingPrime * 2 + 1;
      }
      SORT_SIEVING_PRIME(8)
      SORT_SIEVING_PRIME(9)
      SORT_SIEVING_PRIME(10)
      SORT_SIEVING_PRIME(11)
      SORT_SIEVING_PRIME(12)
      SORT_SIEVING_PRIME(13)
      SORT_SIEVING_PRIME(14)
      SORT_SIEVING_PRIME(15)

      for (;;) // i*30 + 13
      {
        case 16: if (multipleIndex >= sieveSize) goto out16;
                 sieve[multipleIndex] &= BIT2;
                 multipleIndex += sievingPrime * 6 + 2;
        case 17: if (multipleIndex >= sieveSize) goto out17;
                 sieve[multipleIndex] &= BIT7;
                 multipleIndex += sievingPrime * 4 + 2;
        case 18: if (multipleIndex >= sieveSize) goto out18;
                 sieve[multipleIndex] &= BIT5;
                 multipleIndex += sievingPrime * 2 + 1;
        case 19: if (multipleIndex >= sieveSize) goto out19;
                 sieve[multipleIndex] &= BIT4;
                 multipleIndex += sievingPrime * 4 + 2;
        case 20: if (multipleIndex >= sieveSize) goto out20;
                 sieve[multipleIndex] &= BIT1;
                 multipleIndex += sievingPrime * 2 + 1;
        case 21: if (multipleIndex >= sieveSize) goto out21;
                 sieve[multipleIndex] &= BIT0;
                 multipleIndex += sievingPrime * 4 + 1;
        case 22: if (multipleIndex >= sieveSize) goto out22;
                 sieve[multipleIndex] &= BIT6;
                 multipleIndex += sievingPrime * 6 + 3;
        case 23: if (multipleIndex >= sieveSize) goto out23;
                 sieve[multipleIndex] &= BIT3;
                 multipleIndex += sievingPrime * 2 + 1;
      }
      SORT_SIEVING_PRIME(16)
      SORT_SIEVING_PRIME(17)
      SORT_SIEVING_PRIME(18)
      SORT_SIEVING_PRIME(19)
      SORT_SIEVING_PRIME(20)
      SORT_SIEVING_PRIME(21)
      SORT_SIEVING_PRIME(22)
      SORT_SIEVING_PRIME(23)

      for (;;) // i*30 + 17
      {
        case 24: if (multipleIndex >= sieveSize) goto out24;
                 sieve[multipleIndex] &= BIT3;
                 multipleIndex += sievingPrime * 6 + 3;
        case 25: if (multipleIndex >= sieveSize) goto out25;
                 sieve[multipleIndex] &= BIT6;
                 multipleIndex += sievingPrime * 4 + 3;
        case 26: if (multipleIndex >= sieveSize) goto out26;
                 sieve[multipleIndex] &= BIT0;
                 multipleIndex += sievingPrime * 2 + 1;
        case 27: if (multipleIndex >= sieveSize) goto out27;
                 sieve[multipleIndex] &= BIT1;
                 multipleIndex += sievingPrime * 4 + 2;
        case 28: if (multipleIndex >= sieveSize) goto out28;
                 sieve[multipleIndex] &= BIT4;
                 multipleIndex += sievingPrime * 2 + 1;
        case 29: if (multipleIndex >= sieveSize) goto out29;
                 sieve[multipleIndex] &= BIT5;
                 multipleIndex += sievingPrime * 4 + 2;
        case 30: if (multipleIndex >= sieveSize) goto out30;
                 sieve[multipleIndex] &= BIT7;
                 multipleIndex += sievingPrime * 6 + 4;
        case 31: if (multipleIndex >= sieveSize) goto out31;
                 sieve[multipleIndex] &= BIT2;
                 multipleIndex += sievingPrime * 2 + 1;
      }
      SORT_SIEVING_PRIME(24)
      SORT_SIEVING_PRIME(25)
      SORT_SIEVING_PRIME(26)
      SORT_SIEVING_PRIME(27)
      SORT_SIEVING_PRIME(28)
      SORT_SIEVING_PRIME(29)
      SORT_SIEVING_PRIME(30)
      SORT_SIEVING_PRIME(31)

      for (;;) // i*30 + 19
      {
        case 32: if (multipleIndex >= sieveSize) goto out32;
                 sieve[multipleIndex] &= BIT4;
                 multipleIndex += sievingPrime * 6 + 4;
        case 33: if (multipleIndex >= sieveSize) goto out33;
                 sieve[multipleIndex] &= BIT2;
                 multipleIndex += sievingPrime * 4 + 2;
        case 34: if (multipleIndex >= sieveSize) goto out34;
                 sieve[multipleIndex] &= BIT6;
                 multipleIndex += sievingPrime * 2 + 2;
        case 35: if (multipleIndex >= sieveSize) goto out35;
                 sieve[multipleIndex] &= BIT0;
                 multipleIndex += sievingPrime * 4 + 2;
        case 36: if (multipleIndex >= sieveSize) goto out36;
                 sieve[multipleIndex] &= BIT5;
                 multipleIndex += sievingPrime * 2 + 1;
        case 37: if (multipleIndex >= sieveSize) goto out37;
                 sieve[multipleIndex] &= BIT7;
                 multipleIndex += sievingPrime * 4 + 3;
        case 38: if (multipleIndex >= sieveSize) goto out38;
                 sieve[multipleIndex] &= BIT3;
                 multipleIndex += sievingPrime * 6 + 4;
        case 39: if (multipleIndex >= sieveSize) goto out39;
                 sieve[multipleIndex] &= BIT1;
                 multipleIndex += sievingPrime * 2 + 1;
      }
      SORT_SIEVING_PRIME(32)
      SORT_SIEVING_PRIME(33)
      SORT_SIEVING_PRIME(34)
      SORT_SIEVING_PRIME(35)
      SORT_SIEVING_PRIME(36)
      SORT_SIEVING_PRIME(37)
      SORT_SIEVING_PRIME(38)
      SORT_SIEVING_PRIME(39)

      for (;;) // i*30 + 23
      {
        case 40: if (multipleIndex >= sieveSize) goto out40;
                 sieve[multipleIndex] &= BIT5;
                 multipleIndex += sievingPrime * 6 + 5;
        case 41: if (multipleIndex >= sieveSize) goto out41;
                 sieve[multipleIndex] &= BIT1;
                 multipleIndex += sievingPrime * 4 + 3;
        case 42: if (multipleIndex >= sieveSize) goto out42;
                 sieve[multipleIndex] &= BIT2;
                 multipleIndex += sievingPrime * 2 + 1;
        case 43: if (multipleIndex >= sieveSize) goto out43;
                 sieve[multipleIndex] &= BIT6;
                 multipleIndex += sievingPrime * 4 + 3;
        case 44: if (multipleIndex >= sieveSize) goto out44;
                 sieve[multipleIndex] &= BIT7;
                 multipleIndex += sievingPrime * 2 + 2;
        case 45: if (multipleIndex >= sieveSize) goto out45;
                 sieve[multipleIndex] &= BIT3;
                 multipleIndex += sievingPrime * 4 + 3;
        case 46: if (multipleIndex >= sieveSize) goto out46;
                 sieve[multipleIndex] &= BIT4;
                 multipleIndex += sievingPrime * 6 + 5;
        case 47: if (multipleIndex >= sieveSize) goto out47;
                 sieve[multipleIndex] &= BIT0;
                 multipleIndex += sievingPrime * 2 + 1;
      }
      SORT_SIEVING_PRIME(40)
      SORT_SIEVING_PRIME(41)
      SORT_SIEVING_PRIME(42)
      SORT_SIEVING_PRIME(43)
      SORT_SIEVING_PRIME(44)
      SORT_SIEVING_PRIME(45)
      SORT_SIEVING_PRIME(46)
      SORT_SIEVING_PRIME(47)

      for (;;) // i*30 + 29
      {
        case 48: if (multipleIndex >= sieveSize) goto out48;
                 sieve[multipleIndex] &= BIT6;
                 multipleIndex += sievingPrime * 6 + 6;
        case 49: if (multipleIndex >= sieveSize) goto out49;
                 sieve[multipleIndex] &= BIT5;
                 multipleIndex += sievingPrime * 4 + 4;
        case 50: if (multipleIndex >= sieveSize) goto out50;
                 sieve[multipleIndex] &= BIT4;
                 multipleIndex += sievingPrime * 2 + 2;
        case 51: if (multipleIndex >= sieveSize) goto out51;
                 sieve[multipleIndex] &= BIT3;
                 multipleIndex += sievingPrime * 4 + 4;
        case 52: if (multipleIndex >= sieveSize) goto out52;
                 sieve[multipleIndex] &= BIT2;
                 multipleIndex += sievingPrime * 2 + 2;
        case 53: if (multipleIndex >= sieveSize) goto out53;
                 sieve[multipleIndex] &= BIT1;
                 multipleIndex += sievingPrime * 4 + 4;
        case 54: if (multipleIndex >= sieveSize) goto out54;
                 sieve[multipleIndex] &= BIT0;
                 multipleIndex += sievingPrime * 6 + 5;
        case 55: if (multipleIndex >= sieveSize) goto out55;
                 sieve[multipleIndex] &= BIT7;
                 multipleIndex += sievingPrime * 2 + 2;
      }
      SORT_SIEVING_PRIME(48)
      SORT_SIEVING_PRIME(49)
      SORT_SIEVING_PRIME(50)
      SORT_SIEVING_PRIME(51)
      SORT_SIEVING_PRIME(52)
      SORT_SIEVING_PRIME(53)
      SORT_SIEVING_PRIME(54)
      SORT_SIEVING_PRIME(55)

      for (;;) // i*30 + 31
      {
        case 56: if (multipleIndex >= sieveSize) goto out56;
                 sieve[multipleIndex] &= BIT7;
                 multipleIndex += sievingPrime * 6 + 1;
        case 57: if (multipleIndex >= sieveSize) goto out57;
                 sieve[multipleIndex] &= BIT0;
                 multipleIndex += sievingPrime * 4 + 0;
        case 58: if (multipleIndex >= sieveSize) goto out58;
                 sieve[multipleIndex] &= BIT1;
                 multipleIndex += sievingPrime * 2 + 0;
        case 59: if (multipleIndex >= sieveSize) goto out59;
                 sieve[multipleIndex] &= BIT2;
                 multipleIndex += sievingPrime * 4 + 0;
        case 60: if (multipleIndex >= sieveSize) goto out60;
                 sieve[multipleIndex] &= BIT3;
                 multipleIndex += sievingPrime * 2 + 0;
        case 61: if (multipleIndex >= sieveSize) goto out61;
                 sieve[multipleIndex] &= BIT4;
                 multipleIndex += sievingPrime * 4 + 0;
        case 62: if (multipleIndex >= sieveSize) goto out62;
                 sieve[multipleIndex] &= BIT5;
                 multipleIndex += sievingPrime * 6 + 0;
        case 63: if (multipleIndex >= sieveSize) goto out63;
                 sieve[multipleIndex] &= BIT6;
                 multipleIndex += sievingPrime * 2 + 0;
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
