///
/// @file   EratSmall.cpp
/// @brief  Segmented sieve of Eratosthenes optimized for small
///         sieving primes.
///
/// Copyright (C) 2014 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/config.hpp>
#include <primesieve/EratSmall.hpp>
#include <primesieve/WheelFactorization.hpp>
#include <primesieve/primesieve_error.hpp>
#include <primesieve/bits.hpp>

#include <stdint.h>
#include <cassert>
#include <list>

namespace primesieve {

/// @param stop       Upper bound for sieving.
/// @param sieveSize  Sieve size in bytes.
/// @param limit      Sieving primes in EratSmall must be <= limit.
///
EratSmall::EratSmall(uint64_t stop, uint_t sieveSize, uint_t limit) :
  Modulo30Wheel_t(stop, sieveSize),
  limit_(limit)
{
  if (limit > sieveSize * 3)
    throw primesieve_error("EratSmall: limit must be <= sieveSize * 3");
  buckets_.push_back(Bucket());
}

/// Add a new sieving prime to EratSmall
void EratSmall::storeSievingPrime(uint_t prime, uint_t multipleIndex, uint_t wheelIndex)
{
  assert(prime <= limit_);
  uint_t sievingPrime = prime / NUMBERS_PER_BYTE;
  if (!buckets_.back().store(sievingPrime, multipleIndex, wheelIndex))
    buckets_.push_back(Bucket());
}

/// Cross-off the multiples of small sieving
/// primes from the sieve array.
///
void EratSmall::crossOff(byte_t* sieve, byte_t* sieveLimit)
{
  for (BucketIterator_t iter = buckets_.begin(); iter != buckets_.end(); ++iter)
    crossOff(sieve, sieveLimit, *iter);
}

/// Cross-off the multiples of the sieving primes within the current
/// bucket. This is an implementation of the segmented sieve of
/// Eratosthenes with wheel factorization optimized for small sieving
/// primes that have many multiples per segment. This algorithm uses a
/// hardcoded modulo 30 wheel that skips multiples of 2, 3 and 5.
///
void EratSmall::crossOff(byte_t* sieve, byte_t* sieveLimit, Bucket& bucket)
{
  SievingPrime* sPrime = bucket.begin();
  SievingPrime* sEnd   = bucket.end();

  for (; sPrime != sEnd; sPrime++)
  {
    uint_t sievingPrime  = sPrime->getSievingPrime();
    uint_t multipleIndex = sPrime->getMultipleIndex();
    uint_t wheelIndex    = sPrime->getWheelIndex();

    // pointer to the byte containing the first multiple
    // of sievingPrime within the current segment
    byte_t* p = &sieve[multipleIndex];
    byte_t* loopLimit = sieveLimit - (sievingPrime * 28 + 27);
    if (loopLimit > sieveLimit)
      loopLimit = p;

    switch (wheelIndex)
    {
      for (;;) // i*30 + 7
      {
        case 0: // each iteration removes the next 8 multiples
                // of the current sievingPrime
                for (; p < loopLimit; p += sievingPrime * 30 + 7)
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
                if (p >= sieveLimit) { sPrime->setWheelIndex(0); break; }
                *p &= BIT0; p += sievingPrime * 6 + 1;
        case 1: if (p >= sieveLimit) { sPrime->setWheelIndex(1); break; }
                *p &= BIT4; p += sievingPrime * 4 + 1;
        case 2: if (p >= sieveLimit) { sPrime->setWheelIndex(2); break; }
                *p &= BIT3; p += sievingPrime * 2 + 0;
        case 3: if (p >= sieveLimit) { sPrime->setWheelIndex(3); break; }
                *p &= BIT7; p += sievingPrime * 4 + 1;
        case 4: if (p >= sieveLimit) { sPrime->setWheelIndex(4); break; }
                *p &= BIT6; p += sievingPrime * 2 + 1;
        case 5: if (p >= sieveLimit) { sPrime->setWheelIndex(5); break; }
                *p &= BIT2; p += sievingPrime * 4 + 1;
        case 6: if (p >= sieveLimit) { sPrime->setWheelIndex(6); break; }
                *p &= BIT1; p += sievingPrime * 6 + 1;
        case 7: if (p >= sieveLimit) { sPrime->setWheelIndex(7); break; }
                *p &= BIT5; p += sievingPrime * 2 + 1;
      }
      break;
      for (;;) // i*30 + 11
      {
        case  8: for (; p < loopLimit; p += sievingPrime * 30 + 11)
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
                 if (p >= sieveLimit) { sPrime->setWheelIndex(8);  break; }
                 *p &= BIT1; p += sievingPrime * 6 + 2;
        case  9: if (p >= sieveLimit) { sPrime->setWheelIndex(9);  break; }
                 *p &= BIT3; p += sievingPrime * 4 + 1;
        case 10: if (p >= sieveLimit) { sPrime->setWheelIndex(10); break; }
                 *p &= BIT7; p += sievingPrime * 2 + 1;
        case 11: if (p >= sieveLimit) { sPrime->setWheelIndex(11); break; }
                 *p &= BIT5; p += sievingPrime * 4 + 2;
        case 12: if (p >= sieveLimit) { sPrime->setWheelIndex(12); break; }
                 *p &= BIT0; p += sievingPrime * 2 + 0;
        case 13: if (p >= sieveLimit) { sPrime->setWheelIndex(13); break; }
                 *p &= BIT6; p += sievingPrime * 4 + 2;
        case 14: if (p >= sieveLimit) { sPrime->setWheelIndex(14); break; }
                 *p &= BIT2; p += sievingPrime * 6 + 2;
        case 15: if (p >= sieveLimit) { sPrime->setWheelIndex(15); break; }
                 *p &= BIT4; p += sievingPrime * 2 + 1;
      }
      break;
      for (;;) // i*30 + 13
      {
        case 16: for (; p < loopLimit; p += sievingPrime * 30 + 13)
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
                 if (p >= sieveLimit) { sPrime->setWheelIndex(16); break; }
                 *p &= BIT2; p += sievingPrime * 6 + 2;
        case 17: if (p >= sieveLimit) { sPrime->setWheelIndex(17); break; }
                 *p &= BIT7; p += sievingPrime * 4 + 2;
        case 18: if (p >= sieveLimit) { sPrime->setWheelIndex(18); break; }
                 *p &= BIT5; p += sievingPrime * 2 + 1;
        case 19: if (p >= sieveLimit) { sPrime->setWheelIndex(19); break; }
                 *p &= BIT4; p += sievingPrime * 4 + 2;
        case 20: if (p >= sieveLimit) { sPrime->setWheelIndex(20); break; }
                 *p &= BIT1; p += sievingPrime * 2 + 1;
        case 21: if (p >= sieveLimit) { sPrime->setWheelIndex(21); break; }
                 *p &= BIT0; p += sievingPrime * 4 + 1;
        case 22: if (p >= sieveLimit) { sPrime->setWheelIndex(22); break; }
                 *p &= BIT6; p += sievingPrime * 6 + 3;
        case 23: if (p >= sieveLimit) { sPrime->setWheelIndex(23); break; }
                 *p &= BIT3; p += sievingPrime * 2 + 1;
      }
      break;
      for (;;) // i*30 + 17
      {
        case 24: for (; p < loopLimit; p += sievingPrime * 30 + 17)
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
                 if (p >= sieveLimit) { sPrime->setWheelIndex(24); break; }
                 *p &= BIT3; p += sievingPrime * 6 + 3;
        case 25: if (p >= sieveLimit) { sPrime->setWheelIndex(25); break; }
                 *p &= BIT6; p += sievingPrime * 4 + 3;
        case 26: if (p >= sieveLimit) { sPrime->setWheelIndex(26); break; }
                 *p &= BIT0; p += sievingPrime * 2 + 1;
        case 27: if (p >= sieveLimit) { sPrime->setWheelIndex(27); break; }
                 *p &= BIT1; p += sievingPrime * 4 + 2;
        case 28: if (p >= sieveLimit) { sPrime->setWheelIndex(28); break; }
                 *p &= BIT4; p += sievingPrime * 2 + 1;
        case 29: if (p >= sieveLimit) { sPrime->setWheelIndex(29); break; }
                 *p &= BIT5; p += sievingPrime * 4 + 2;
        case 30: if (p >= sieveLimit) { sPrime->setWheelIndex(30); break; }
                 *p &= BIT7; p += sievingPrime * 6 + 4;
        case 31: if (p >= sieveLimit) { sPrime->setWheelIndex(31); break; }
                 *p &= BIT2; p += sievingPrime * 2 + 1;
      }
      break;
      for (;;) // i*30 + 19
      {
        case 32: for (; p < loopLimit; p += sievingPrime * 30 + 19)
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
                 if (p >= sieveLimit) { sPrime->setWheelIndex(32); break; }
                 *p &= BIT4; p += sievingPrime * 6 + 4;
        case 33: if (p >= sieveLimit) { sPrime->setWheelIndex(33); break; }
                 *p &= BIT2; p += sievingPrime * 4 + 2;
        case 34: if (p >= sieveLimit) { sPrime->setWheelIndex(34); break; }
                 *p &= BIT6; p += sievingPrime * 2 + 2;
        case 35: if (p >= sieveLimit) { sPrime->setWheelIndex(35); break; }
                 *p &= BIT0; p += sievingPrime * 4 + 2;
        case 36: if (p >= sieveLimit) { sPrime->setWheelIndex(36); break; }
                 *p &= BIT5; p += sievingPrime * 2 + 1;
        case 37: if (p >= sieveLimit) { sPrime->setWheelIndex(37); break; }
                 *p &= BIT7; p += sievingPrime * 4 + 3;
        case 38: if (p >= sieveLimit) { sPrime->setWheelIndex(38); break; }
                 *p &= BIT3; p += sievingPrime * 6 + 4;
        case 39: if (p >= sieveLimit) { sPrime->setWheelIndex(39); break; }
                 *p &= BIT1; p += sievingPrime * 2 + 1;
      }
      break;
      for (;;) // i*30 + 23
      {
        case 40: for (; p < loopLimit; p += sievingPrime * 30 + 23)
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
                 if (p >= sieveLimit) { sPrime->setWheelIndex(40); break; }
                 *p &= BIT5; p += sievingPrime * 6 + 5;
        case 41: if (p >= sieveLimit) { sPrime->setWheelIndex(41); break; }
                 *p &= BIT1; p += sievingPrime * 4 + 3;
        case 42: if (p >= sieveLimit) { sPrime->setWheelIndex(42); break; }
                 *p &= BIT2; p += sievingPrime * 2 + 1;
        case 43: if (p >= sieveLimit) { sPrime->setWheelIndex(43); break; }
                 *p &= BIT6; p += sievingPrime * 4 + 3;
        case 44: if (p >= sieveLimit) { sPrime->setWheelIndex(44); break; }
                 *p &= BIT7; p += sievingPrime * 2 + 2;
        case 45: if (p >= sieveLimit) { sPrime->setWheelIndex(45); break; }
                 *p &= BIT3; p += sievingPrime * 4 + 3;
        case 46: if (p >= sieveLimit) { sPrime->setWheelIndex(46); break; }
                 *p &= BIT4; p += sievingPrime * 6 + 5;
        case 47: if (p >= sieveLimit) { sPrime->setWheelIndex(47); break; }
                 *p &= BIT0; p += sievingPrime * 2 + 1;
      }
      break;
      for (;;) // i*30 + 29
      {
        case 48: for (; p < loopLimit; p += sievingPrime * 30 + 29)
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
                 if (p >= sieveLimit) { sPrime->setWheelIndex(48); break; }
                 *p &= BIT6; p += sievingPrime * 6 + 6;
        case 49: if (p >= sieveLimit) { sPrime->setWheelIndex(49); break; }
                 *p &= BIT5; p += sievingPrime * 4 + 4;
        case 50: if (p >= sieveLimit) { sPrime->setWheelIndex(50); break; }
                 *p &= BIT4; p += sievingPrime * 2 + 2;
        case 51: if (p >= sieveLimit) { sPrime->setWheelIndex(51); break; }
                 *p &= BIT3; p += sievingPrime * 4 + 4;
        case 52: if (p >= sieveLimit) { sPrime->setWheelIndex(52); break; }
                 *p &= BIT2; p += sievingPrime * 2 + 2;
        case 53: if (p >= sieveLimit) { sPrime->setWheelIndex(53); break; }
                 *p &= BIT1; p += sievingPrime * 4 + 4;
        case 54: if (p >= sieveLimit) { sPrime->setWheelIndex(54); break; }
                 *p &= BIT0; p += sievingPrime * 6 + 5;
        case 55: if (p >= sieveLimit) { sPrime->setWheelIndex(55); break; }
                 *p &= BIT7; p += sievingPrime * 2 + 2;
      }
      break;
      for (;;) // i*30 + 31
      {
        case 56: for (; p < loopLimit; p += sievingPrime * 30 + 1)
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
                 if (p >= sieveLimit) { sPrime->setWheelIndex(56); break; }
                 *p &= BIT7; p += sievingPrime * 6 + 1;
        case 57: if (p >= sieveLimit) { sPrime->setWheelIndex(57); break; }
                 *p &= BIT0; p += sievingPrime * 4 + 0;
        case 58: if (p >= sieveLimit) { sPrime->setWheelIndex(58); break; }
                 *p &= BIT1; p += sievingPrime * 2 + 0;
        case 59: if (p >= sieveLimit) { sPrime->setWheelIndex(59); break; }
                 *p &= BIT2; p += sievingPrime * 4 + 0;
        case 60: if (p >= sieveLimit) { sPrime->setWheelIndex(60); break; }
                 *p &= BIT3; p += sievingPrime * 2 + 0;
        case 61: if (p >= sieveLimit) { sPrime->setWheelIndex(61); break; }
                 *p &= BIT4; p += sievingPrime * 4 + 0;
        case 62: if (p >= sieveLimit) { sPrime->setWheelIndex(62); break; }
                 *p &= BIT5; p += sievingPrime * 6 + 0;
        case 63: if (p >= sieveLimit) { sPrime->setWheelIndex(63); break; }
                 *p &= BIT6; p += sievingPrime * 2 + 0;
      }
      break;
    }
    // set multipleIndex for the next segment
    sPrime->setMultipleIndex(static_cast<uint_t>(p - sieveLimit));
  }
}

} // namespace primesieve
