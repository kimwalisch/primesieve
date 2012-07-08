//
// Copyright (c) 2012 Kim Walisch, <kim.walisch@gmail.com>.
// All rights reserved.
//
// This file is part of primesieve.
// Homepage: http://primesieve.googlecode.com
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//   * Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above
//     copyright notice, this list of conditions and the following
//     disclaimer in the documentation and/or other materials provided
//     with the distribution.
//   * Neither the name of the author nor the names of its
//     contributors may be used to endorse or promote products derived
//     from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.

#include "EratSmall.h"
#include "SieveOfEratosthenes.h"
#include "SieveOfEratosthenes-inline.h"
#include "WheelFactorization.h"
#include "config.h"
#include "bits.h"

#include <stdint.h>
#include <cassert>
#include <algorithm>
#include <list>

namespace soe {

EratSmall::EratSmall(const SieveOfEratosthenes& soe) :
  Modulo30Wheel_t(soe), buckets_(1, Bucket())
{
  // assert multipleIndex < 2^23 in crossOff()
  assert(config::FACTOR_ERATSMALL <= 4.0);
  uint_t max = static_cast<uint_t>(soe.getSieveSize() * config::FACTOR_ERATSMALL);
  limit_ = std::min(soe.getSqrtStop(), max);
}

/// Add a new sieving prime
/// @see addSievingPrime() in WheelFactorization.h
///
void EratSmall::storeSievingPrime(uint_t sievingPrime, uint_t multipleIndex, uint_t wheelIndex)
{
  if (!buckets_.back().store(sievingPrime, multipleIndex, wheelIndex))
    buckets_.push_back(Bucket());
}

/// Cross-off the multiples of sieving primes wihtin EratSmall
/// @see crossOffMultiples() in SieveOfEratosthenes.cpp
///
void EratSmall::crossOff(uint8_t* sieve, uint8_t* sieveLimit)
{
  for (BucketIterator_t bucket = buckets_.begin(); bucket != buckets_.end(); ++bucket)
    crossOff(*bucket, sieve, sieveLimit);
}

/// Cross-off the multiples of the sieving primes within the current
/// bucket. This is an implementation of the segmented sieve of
/// Eratosthenes with wheel factorization optimized for small sieving
/// primes that have many multiples per segment. This algorithm uses a
/// hardcoded modulo 30 wheel that skips multiples of 2, 3 and 5.
///
void EratSmall::crossOff(Bucket& bucket, uint8_t* sieve, uint8_t* sieveLimit)
{
  WheelPrime* wPrime = bucket.begin();
  WheelPrime* end    = bucket.end();

  for (; wPrime != end; wPrime++) {
    uint_t sievingPrime  = wPrime->getSievingPrime();
    uint_t multipleIndex = wPrime->getMultipleIndex();
    uint_t wheelIndex    = wPrime->getWheelIndex();

    uint8_t* loopLimit = std::max(sieve, sieveLimit - (sievingPrime * 28 + 27));

    // pointer to the byte containing the first multiple of
    // sievingPrime within the current segment
    uint8_t* p = &sieve[multipleIndex];

    // cross-off the multiples (unset bits) of sievingPrime
    switch (wheelIndex) {
      // for sieving primes of type i*30 + 7
      for (;;) {
        case 0: // fast loop, uses only =~ 12 asm instructions (x86-64)
                // per iteration to cross-off the next 8 multiples
                for (; p < loopLimit; p += sievingPrime * 30 + 7) {
                  p[sievingPrime *  0 + 0] &= BIT0;
                  p[sievingPrime *  6 + 1] &= BIT4;
                  p[sievingPrime * 10 + 2] &= BIT3;
                  p[sievingPrime * 12 + 2] &= BIT7;
                  p[sievingPrime * 16 + 3] &= BIT6;
                  p[sievingPrime * 18 + 4] &= BIT2;
                  p[sievingPrime * 22 + 5] &= BIT1;
                  p[sievingPrime * 28 + 6] &= BIT5;
                }
                if (p >= sieveLimit) { wPrime->setWheelIndex(0); break; } *p &= BIT0; p += sievingPrime * 6 + 1;
        case 1: if (p >= sieveLimit) { wPrime->setWheelIndex(1); break; } *p &= BIT4; p += sievingPrime * 4 + 1;
        case 2: if (p >= sieveLimit) { wPrime->setWheelIndex(2); break; } *p &= BIT3; p += sievingPrime * 2 + 0;
        case 3: if (p >= sieveLimit) { wPrime->setWheelIndex(3); break; } *p &= BIT7; p += sievingPrime * 4 + 1;
        case 4: if (p >= sieveLimit) { wPrime->setWheelIndex(4); break; } *p &= BIT6; p += sievingPrime * 2 + 1;
        case 5: if (p >= sieveLimit) { wPrime->setWheelIndex(5); break; } *p &= BIT2; p += sievingPrime * 4 + 1;
        case 6: if (p >= sieveLimit) { wPrime->setWheelIndex(6); break; } *p &= BIT1; p += sievingPrime * 6 + 1;
        case 7: if (p >= sieveLimit) { wPrime->setWheelIndex(7); break; } *p &= BIT5; p += sievingPrime * 2 + 1;
      }
      break;
      // for sieving primes of type i*30 + 11
      for (;;) {
        case  8: for (; p < loopLimit; p += sievingPrime * 30 + 11) {
                   p[sievingPrime *  0 +  0] &= BIT1;
                   p[sievingPrime *  6 +  2] &= BIT3;
                   p[sievingPrime * 10 +  3] &= BIT7;
                   p[sievingPrime * 12 +  4] &= BIT5;
                   p[sievingPrime * 16 +  6] &= BIT0;
                   p[sievingPrime * 18 +  6] &= BIT6;
                   p[sievingPrime * 22 +  8] &= BIT2; 
                   p[sievingPrime * 28 + 10] &= BIT4;
                 }
                 if (p >= sieveLimit) { wPrime->setWheelIndex(8);  break; } *p &= BIT1; p += sievingPrime * 6 + 2;
        case  9: if (p >= sieveLimit) { wPrime->setWheelIndex(9);  break; } *p &= BIT3; p += sievingPrime * 4 + 1;
        case 10: if (p >= sieveLimit) { wPrime->setWheelIndex(10); break; } *p &= BIT7; p += sievingPrime * 2 + 1;
        case 11: if (p >= sieveLimit) { wPrime->setWheelIndex(11); break; } *p &= BIT5; p += sievingPrime * 4 + 2;
        case 12: if (p >= sieveLimit) { wPrime->setWheelIndex(12); break; } *p &= BIT0; p += sievingPrime * 2 + 0;
        case 13: if (p >= sieveLimit) { wPrime->setWheelIndex(13); break; } *p &= BIT6; p += sievingPrime * 4 + 2;
        case 14: if (p >= sieveLimit) { wPrime->setWheelIndex(14); break; } *p &= BIT2; p += sievingPrime * 6 + 2;
        case 15: if (p >= sieveLimit) { wPrime->setWheelIndex(15); break; } *p &= BIT4; p += sievingPrime * 2 + 1;
      }
      break;
      // for sieving primes of type i*30 + 13
      for (;;) {
        case 16: for (; p < loopLimit; p += sievingPrime * 30 + 13) {
                   p[sievingPrime *  0 +  0] &= BIT2;
                   p[sievingPrime *  6 +  2] &= BIT7;
                   p[sievingPrime * 10 +  4] &= BIT5;
                   p[sievingPrime * 12 +  5] &= BIT4;
                   p[sievingPrime * 16 +  7] &= BIT1;
                   p[sievingPrime * 18 +  8] &= BIT0;
                   p[sievingPrime * 22 +  9] &= BIT6;
                   p[sievingPrime * 28 + 12] &= BIT3;
                 }
                 if (p >= sieveLimit) { wPrime->setWheelIndex(16); break; } *p &= BIT2; p += sievingPrime * 6 + 2;
        case 17: if (p >= sieveLimit) { wPrime->setWheelIndex(17); break; } *p &= BIT7; p += sievingPrime * 4 + 2;
        case 18: if (p >= sieveLimit) { wPrime->setWheelIndex(18); break; } *p &= BIT5; p += sievingPrime * 2 + 1;
        case 19: if (p >= sieveLimit) { wPrime->setWheelIndex(19); break; } *p &= BIT4; p += sievingPrime * 4 + 2;
        case 20: if (p >= sieveLimit) { wPrime->setWheelIndex(20); break; } *p &= BIT1; p += sievingPrime * 2 + 1;
        case 21: if (p >= sieveLimit) { wPrime->setWheelIndex(21); break; } *p &= BIT0; p += sievingPrime * 4 + 1;
        case 22: if (p >= sieveLimit) { wPrime->setWheelIndex(22); break; } *p &= BIT6; p += sievingPrime * 6 + 3;
        case 23: if (p >= sieveLimit) { wPrime->setWheelIndex(23); break; } *p &= BIT3; p += sievingPrime * 2 + 1;
      }
      break;
      // for sieving primes of type i*30 + 17
      for (;;) {
        case 24: for (; p < loopLimit; p += sievingPrime * 30 + 17) {
                   p[sievingPrime *  0 +  0] &= BIT3;
                   p[sievingPrime *  6 +  3] &= BIT6;
                   p[sievingPrime * 10 +  6] &= BIT0;
                   p[sievingPrime * 12 +  7] &= BIT1;
                   p[sievingPrime * 16 +  9] &= BIT4;
                   p[sievingPrime * 18 + 10] &= BIT5;
                   p[sievingPrime * 22 + 12] &= BIT7;
                   p[sievingPrime * 28 + 16] &= BIT2;
                 }
                 if (p >= sieveLimit) { wPrime->setWheelIndex(24); break; } *p &= BIT3; p += sievingPrime * 6 + 3;
        case 25: if (p >= sieveLimit) { wPrime->setWheelIndex(25); break; } *p &= BIT6; p += sievingPrime * 4 + 3;
        case 26: if (p >= sieveLimit) { wPrime->setWheelIndex(26); break; } *p &= BIT0; p += sievingPrime * 2 + 1;
        case 27: if (p >= sieveLimit) { wPrime->setWheelIndex(27); break; } *p &= BIT1; p += sievingPrime * 4 + 2;
        case 28: if (p >= sieveLimit) { wPrime->setWheelIndex(28); break; } *p &= BIT4; p += sievingPrime * 2 + 1;
        case 29: if (p >= sieveLimit) { wPrime->setWheelIndex(29); break; } *p &= BIT5; p += sievingPrime * 4 + 2;
        case 30: if (p >= sieveLimit) { wPrime->setWheelIndex(30); break; } *p &= BIT7; p += sievingPrime * 6 + 4;
        case 31: if (p >= sieveLimit) { wPrime->setWheelIndex(31); break; } *p &= BIT2; p += sievingPrime * 2 + 1;
      }
      break;
      // for sieving primes of type i*30 + 19
      for (;;) {
        case 32: for (; p < loopLimit; p += sievingPrime * 30 + 19) {
                   p[sievingPrime *  0 +  0] &= BIT4;
                   p[sievingPrime *  6 +  4] &= BIT2;
                   p[sievingPrime * 10 +  6] &= BIT6;
                   p[sievingPrime * 12 +  8] &= BIT0;
                   p[sievingPrime * 16 + 10] &= BIT5;
                   p[sievingPrime * 18 + 11] &= BIT7;
                   p[sievingPrime * 22 + 14] &= BIT3;
                   p[sievingPrime * 28 + 18] &= BIT1;
                 }
                 if (p >= sieveLimit) { wPrime->setWheelIndex(32); break; } *p &= BIT4; p += sievingPrime * 6 + 4;
        case 33: if (p >= sieveLimit) { wPrime->setWheelIndex(33); break; } *p &= BIT2; p += sievingPrime * 4 + 2;
        case 34: if (p >= sieveLimit) { wPrime->setWheelIndex(34); break; } *p &= BIT6; p += sievingPrime * 2 + 2;
        case 35: if (p >= sieveLimit) { wPrime->setWheelIndex(35); break; } *p &= BIT0; p += sievingPrime * 4 + 2;
        case 36: if (p >= sieveLimit) { wPrime->setWheelIndex(36); break; } *p &= BIT5; p += sievingPrime * 2 + 1;
        case 37: if (p >= sieveLimit) { wPrime->setWheelIndex(37); break; } *p &= BIT7; p += sievingPrime * 4 + 3;
        case 38: if (p >= sieveLimit) { wPrime->setWheelIndex(38); break; } *p &= BIT3; p += sievingPrime * 6 + 4;
        case 39: if (p >= sieveLimit) { wPrime->setWheelIndex(39); break; } *p &= BIT1; p += sievingPrime * 2 + 1;
      }
      break;
      // for sieving primes of type i*30 + 23
      for (;;) {
        case 40: for (; p < loopLimit; p += sievingPrime * 30 + 23) {
                   p[sievingPrime *  0 +  0] &= BIT5;
                   p[sievingPrime *  6 +  5] &= BIT1;
                   p[sievingPrime * 10 +  8] &= BIT2;
                   p[sievingPrime * 12 +  9] &= BIT6;
                   p[sievingPrime * 16 + 12] &= BIT7;
                   p[sievingPrime * 18 + 14] &= BIT3;
                   p[sievingPrime * 22 + 17] &= BIT4;
                   p[sievingPrime * 28 + 22] &= BIT0;
                 }
                 if (p >= sieveLimit) { wPrime->setWheelIndex(40); break; } *p &= BIT5; p += sievingPrime * 6 + 5;
        case 41: if (p >= sieveLimit) { wPrime->setWheelIndex(41); break; } *p &= BIT1; p += sievingPrime * 4 + 3;
        case 42: if (p >= sieveLimit) { wPrime->setWheelIndex(42); break; } *p &= BIT2; p += sievingPrime * 2 + 1;
        case 43: if (p >= sieveLimit) { wPrime->setWheelIndex(43); break; } *p &= BIT6; p += sievingPrime * 4 + 3;
        case 44: if (p >= sieveLimit) { wPrime->setWheelIndex(44); break; } *p &= BIT7; p += sievingPrime * 2 + 2;
        case 45: if (p >= sieveLimit) { wPrime->setWheelIndex(45); break; } *p &= BIT3; p += sievingPrime * 4 + 3;
        case 46: if (p >= sieveLimit) { wPrime->setWheelIndex(46); break; } *p &= BIT4; p += sievingPrime * 6 + 5;
        case 47: if (p >= sieveLimit) { wPrime->setWheelIndex(47); break; } *p &= BIT0; p += sievingPrime * 2 + 1;
      }
      break;
      // for sieving primes of type i*30 + 29
      for (;;) {
        case 48: for (; p < loopLimit; p += sievingPrime * 30 + 29) {
                   p[sievingPrime *  0 +  0] &= BIT6;
                   p[sievingPrime *  6 +  6] &= BIT5;
                   p[sievingPrime * 10 + 10] &= BIT4;
                   p[sievingPrime * 12 + 12] &= BIT3;
                   p[sievingPrime * 16 + 16] &= BIT2;
                   p[sievingPrime * 18 + 18] &= BIT1;
                   p[sievingPrime * 22 + 22] &= BIT0;
                   p[sievingPrime * 28 + 27] &= BIT7;
                 }
                 if (p >= sieveLimit) { wPrime->setWheelIndex(48); break; } *p &= BIT6; p += sievingPrime * 6 + 6;
        case 49: if (p >= sieveLimit) { wPrime->setWheelIndex(49); break; } *p &= BIT5; p += sievingPrime * 4 + 4;
        case 50: if (p >= sieveLimit) { wPrime->setWheelIndex(50); break; } *p &= BIT4; p += sievingPrime * 2 + 2;
        case 51: if (p >= sieveLimit) { wPrime->setWheelIndex(51); break; } *p &= BIT3; p += sievingPrime * 4 + 4;
        case 52: if (p >= sieveLimit) { wPrime->setWheelIndex(52); break; } *p &= BIT2; p += sievingPrime * 2 + 2;
        case 53: if (p >= sieveLimit) { wPrime->setWheelIndex(53); break; } *p &= BIT1; p += sievingPrime * 4 + 4;
        case 54: if (p >= sieveLimit) { wPrime->setWheelIndex(54); break; } *p &= BIT0; p += sievingPrime * 6 + 5;
        case 55: if (p >= sieveLimit) { wPrime->setWheelIndex(55); break; } *p &= BIT7; p += sievingPrime * 2 + 2;
      }
      break;
      // for sieving primes of type i*30 + 31
      for (;;) {
        case 56: for (; p < loopLimit; p += sievingPrime * 30 + 1) {
                   p[sievingPrime *  0 + 0] &= BIT7;
                   p[sievingPrime *  6 + 1] &= BIT0;
                   p[sievingPrime * 10 + 1] &= BIT1;
                   p[sievingPrime * 12 + 1] &= BIT2;
                   p[sievingPrime * 16 + 1] &= BIT3;
                   p[sievingPrime * 18 + 1] &= BIT4;
                   p[sievingPrime * 22 + 1] &= BIT5;
                   p[sievingPrime * 28 + 1] &= BIT6;
                 }
                 if (p >= sieveLimit) { wPrime->setWheelIndex(56); break; } *p &= BIT7; p += sievingPrime * 6 + 1;
        case 57: if (p >= sieveLimit) { wPrime->setWheelIndex(57); break; } *p &= BIT0; p += sievingPrime * 4 + 0;
        case 58: if (p >= sieveLimit) { wPrime->setWheelIndex(58); break; } *p &= BIT1; p += sievingPrime * 2 + 0;
        case 59: if (p >= sieveLimit) { wPrime->setWheelIndex(59); break; } *p &= BIT2; p += sievingPrime * 4 + 0;
        case 60: if (p >= sieveLimit) { wPrime->setWheelIndex(60); break; } *p &= BIT3; p += sievingPrime * 2 + 0;
        case 61: if (p >= sieveLimit) { wPrime->setWheelIndex(61); break; } *p &= BIT4; p += sievingPrime * 4 + 0;
        case 62: if (p >= sieveLimit) { wPrime->setWheelIndex(62); break; } *p &= BIT5; p += sievingPrime * 6 + 0;
        case 63: if (p >= sieveLimit) { wPrime->setWheelIndex(63); break; } *p &= BIT6; p += sievingPrime * 2 + 0;
      }
      break;
    }
    // set multipleIndex for the next segment
    wPrime->setMultipleIndex(static_cast<uint_t>(p - sieveLimit));
  }
}

} // namespace soe
