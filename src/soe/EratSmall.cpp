//
// Copyright (c) 2011 Kim Walisch, <kim.walisch@gmail.com>.
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
#include "EratBase.h"
#include "WheelFactorization.h"
#include "defs.h"

#include <algorithm>
#include <stdexcept>
#include <list>

EratSmall::EratSmall(const SieveOfEratosthenes& soe) :
  EratBase<Modulo30Wheel_t> (soe)
{
  uint32_t sqrtStop = soe.getSquareRoot();
  uint32_t max      = static_cast<uint32_t> (soe.getSieveSize() * defs::ERATSMALL_FACTOR);
  uint32_t limit    = std::min(sqrtStop, max);
  // sieveSize - 1 + (prime / 30) * 6 + 6 - sieveSize < sieveSize
  // prevents segmentation faults in sieve(uint8_t*, uint32_t)
  if (limit >= (soe.getSieveSize() - 5) * 5)
    throw std::invalid_argument(
        "EratSmall: limit must be < (sieveSize - 5) * 5.");
  this->setLimit(limit);
}

/**
 * Implementation of the segmented sieve of Eratosthenes with wheel
 * factorization optimized for small sieving primes that have many
 * multiples per segment.
 * This implementation uses a sieve array with 30 numbers per byte and
 * a hardcoded modulo 30 wheel that skips multiples of 2, 3 and 5.
 * @see SieveOfEratosthenes::crossOffMultiples()
 */
void EratSmall::sieve(uint8_t* sieve, uint32_t sieveSize) {
  uint8_t* const sieveEnd = &sieve[sieveSize];

  for (BucketList_t::iterator bucket = buckets_.begin(); bucket != buckets_.end(); ++bucket) {
    WheelPrime* wPrime = bucket->begin();
    WheelPrime* end    = bucket->end();
    // process the sieving primes within the current bucket
    for (; wPrime != end; wPrime++) {
      const uint32_t primeX2       = wPrime->getSievingPrime() * 2;
      const uint32_t primeX4       = primeX2 * 2;
      const uint32_t primeX6       = primeX2 * 3;
      const uint32_t maxLoopOffset = primeX2 * 15 + 29;
      const uint32_t multipleIndex = wPrime->getMultipleIndex();
      const uint32_t wheelIndex    = wPrime->getWheelIndex();

      uint8_t* const loopLimit = (maxLoopOffset < sieveSize) ? sieveEnd - maxLoopOffset : sieve;
      uint8_t* s = &sieve[multipleIndex];

      // cross-off the multiples (unset corresponding bits) of the
      // current sieving prime within the sieve array
      switch (wheelIndex) {
        // for sieving primes of type i * 30 + 7
        for (;;) {
          case 1: *s &= BIT0; s += primeX6 + 1; if (s >= sieveEnd) { wPrime->setWheelIndex(2); break; }
          case 2: *s &= BIT4; s += primeX4 + 1; if (s >= sieveEnd) { wPrime->setWheelIndex(3); break; }
          case 3: *s &= BIT3; s += primeX2 + 0; if (s >= sieveEnd) { wPrime->setWheelIndex(4); break; }
          case 4: *s &= BIT7; s += primeX4 + 1; if (s >= sieveEnd) { wPrime->setWheelIndex(5); break; }
          case 5: *s &= BIT6; s += primeX2 + 1; if (s >= sieveEnd) { wPrime->setWheelIndex(6); break; }
          case 6: *s &= BIT2; s += primeX4 + 1; if (s >= sieveEnd) { wPrime->setWheelIndex(7); break; }
          case 7: *s &= BIT1; s += primeX6 + 1; if (s >= sieveEnd) { wPrime->setWheelIndex(0); break; }
          case 0: *s &= BIT5; s += primeX2 + 1;
                  while (s < loopLimit) {
                    *s              &= BIT0; s += primeX6;
                     s[1]           &= BIT4;
                     s[primeX4 + 2] &= BIT3; s += primeX6;
                     s[2]           &= BIT7;
                     s[primeX4 + 3] &= BIT6; s += primeX6;
                     s[4]           &= BIT2;
                     s[primeX4 + 5] &= BIT1; s += primeX6;
                     s[primeX4 + 6] &= BIT5; s += primeX6 + 7;
                  }
                  if (s >= sieveEnd) { wPrime->setWheelIndex(1); break; }
        }
        break;
        // for sieving primes of type i * 30 + 11
        for (;;) {
          case  9: *s &= BIT1; s += primeX6 + 2; if (s >= sieveEnd) { wPrime->setWheelIndex(10); break; }
          case 10: *s &= BIT3; s += primeX4 + 1; if (s >= sieveEnd) { wPrime->setWheelIndex(11); break; }
          case 11: *s &= BIT7; s += primeX2 + 1; if (s >= sieveEnd) { wPrime->setWheelIndex(12); break; }
          case 12: *s &= BIT5; s += primeX4 + 2; if (s >= sieveEnd) { wPrime->setWheelIndex(13); break; }
          case 13: *s &= BIT0; s += primeX2 + 0; if (s >= sieveEnd) { wPrime->setWheelIndex(14); break; }
          case 14: *s &= BIT6; s += primeX4 + 2; if (s >= sieveEnd) { wPrime->setWheelIndex(15); break; }
          case 15: *s &= BIT2; s += primeX6 + 2; if (s >= sieveEnd) { wPrime->setWheelIndex(8);  break; }
          case  8: *s &= BIT4; s += primeX2 + 1;
                   while (s < loopLimit) {
                     *s               &= BIT1; s += primeX6;
                      s[2]            &= BIT3;
                      s[primeX4 +  3] &= BIT7; s += primeX6;
                      s[4]            &= BIT5;
                      s[primeX4 +  6] &= BIT0; s += primeX6;
                      s[6]            &= BIT6;
                      s[primeX4 +  8] &= BIT2; s += primeX6;
                      s[primeX4 + 10] &= BIT4; s += primeX6 + 11;
                   }
                   if (s >= sieveEnd) { wPrime->setWheelIndex(9); break; }
        }
        break;
        // for sieving primes of type i * 30 + 13
        for (;;) {
          case 17: *s &= BIT2; s += primeX6 + 2; if (s >= sieveEnd) { wPrime->setWheelIndex(18); break; }
          case 18: *s &= BIT7; s += primeX4 + 2; if (s >= sieveEnd) { wPrime->setWheelIndex(19); break; }
          case 19: *s &= BIT5; s += primeX2 + 1; if (s >= sieveEnd) { wPrime->setWheelIndex(20); break; }
          case 20: *s &= BIT4; s += primeX4 + 2; if (s >= sieveEnd) { wPrime->setWheelIndex(21); break; }
          case 21: *s &= BIT1; s += primeX2 + 1; if (s >= sieveEnd) { wPrime->setWheelIndex(22); break; }
          case 22: *s &= BIT0; s += primeX4 + 1; if (s >= sieveEnd) { wPrime->setWheelIndex(23); break; }
          case 23: *s &= BIT6; s += primeX6 + 3; if (s >= sieveEnd) { wPrime->setWheelIndex(16); break; }
          case 16: *s &= BIT3; s += primeX2 + 1;
                   while (s < loopLimit) {
                     *s               &= BIT2; s += primeX6;
                      s[2]            &= BIT7;
                      s[primeX4 +  4] &= BIT5; s += primeX6;
                      s[5]            &= BIT4;
                      s[primeX4 +  7] &= BIT1; s += primeX6;
                      s[8]            &= BIT0;
                      s[primeX4 +  9] &= BIT6; s += primeX6;
                      s[primeX4 + 12] &= BIT3; s += primeX6 + 13;
                   }
                   if (s >= sieveEnd) { wPrime->setWheelIndex(17); break; }
        }
        break;
        // for sieving primes of type i * 30 + 17
        for (;;) {
          case 25: *s &= BIT3; s += primeX6 + 3; if (s >= sieveEnd) { wPrime->setWheelIndex(26); break; }
          case 26: *s &= BIT6; s += primeX4 + 3; if (s >= sieveEnd) { wPrime->setWheelIndex(27); break; }
          case 27: *s &= BIT0; s += primeX2 + 1; if (s >= sieveEnd) { wPrime->setWheelIndex(28); break; }
          case 28: *s &= BIT1; s += primeX4 + 2; if (s >= sieveEnd) { wPrime->setWheelIndex(29); break; }
          case 29: *s &= BIT4; s += primeX2 + 1; if (s >= sieveEnd) { wPrime->setWheelIndex(30); break; }
          case 30: *s &= BIT5; s += primeX4 + 2; if (s >= sieveEnd) { wPrime->setWheelIndex(31); break; }
          case 31: *s &= BIT7; s += primeX6 + 4; if (s >= sieveEnd) { wPrime->setWheelIndex(24); break; }
          case 24: *s &= BIT2; s += primeX2 + 1;
                   while (s < loopLimit) {
                     *s               &= BIT3; s += primeX6;
                      s[3]            &= BIT6;
                      s[primeX4 +  6] &= BIT0; s += primeX6;
                      s[7]            &= BIT1;
                      s[primeX4 +  9] &= BIT4; s += primeX6;
                      s[10]           &= BIT5;
                      s[primeX4 + 12] &= BIT7; s += primeX6;
                      s[primeX4 + 16] &= BIT2; s += primeX6 + 17;
                   }
                   if (s >= sieveEnd) { wPrime->setWheelIndex(25); break; }
        }
        break;
        // for sieving primes of type i * 30 + 19
        for (;;) {
          case 33: *s &= BIT4; s += primeX6 + 4; if (s >= sieveEnd) { wPrime->setWheelIndex(34); break; }
          case 34: *s &= BIT2; s += primeX4 + 2; if (s >= sieveEnd) { wPrime->setWheelIndex(35); break; }
          case 35: *s &= BIT6; s += primeX2 + 2; if (s >= sieveEnd) { wPrime->setWheelIndex(36); break; }
          case 36: *s &= BIT0; s += primeX4 + 2; if (s >= sieveEnd) { wPrime->setWheelIndex(37); break; }
          case 37: *s &= BIT5; s += primeX2 + 1; if (s >= sieveEnd) { wPrime->setWheelIndex(38); break; }
          case 38: *s &= BIT7; s += primeX4 + 3; if (s >= sieveEnd) { wPrime->setWheelIndex(39); break; }
          case 39: *s &= BIT3; s += primeX6 + 4; if (s >= sieveEnd) { wPrime->setWheelIndex(32); break; }
          case 32: *s &= BIT1; s += primeX2 + 1;
                   while (s < loopLimit) {
                     *s               &= BIT4; s += primeX6;
                      s[4]            &= BIT2;
                      s[primeX4 +  6] &= BIT6; s += primeX6;
                      s[8]            &= BIT0;
                      s[primeX4 + 10] &= BIT5; s += primeX6;
                      s[11]           &= BIT7;
                      s[primeX4 + 14] &= BIT3; s += primeX6;
                      s[primeX4 + 18] &= BIT1; s += primeX6 + 19;
                   }
                   if (s >= sieveEnd) { wPrime->setWheelIndex(33); break; }
        }
        break;
        // for sieving primes of type i * 30 + 23
        for (;;) {
          case 41: *s &= BIT5; s += primeX6 + 5; if (s >= sieveEnd) { wPrime->setWheelIndex(42); break; }
          case 42: *s &= BIT1; s += primeX4 + 3; if (s >= sieveEnd) { wPrime->setWheelIndex(43); break; }
          case 43: *s &= BIT2; s += primeX2 + 1; if (s >= sieveEnd) { wPrime->setWheelIndex(44); break; }
          case 44: *s &= BIT6; s += primeX4 + 3; if (s >= sieveEnd) { wPrime->setWheelIndex(45); break; }
          case 45: *s &= BIT7; s += primeX2 + 2; if (s >= sieveEnd) { wPrime->setWheelIndex(46); break; }
          case 46: *s &= BIT3; s += primeX4 + 3; if (s >= sieveEnd) { wPrime->setWheelIndex(47); break; }
          case 47: *s &= BIT4; s += primeX6 + 5; if (s >= sieveEnd) { wPrime->setWheelIndex(40); break; }
          case 40: *s &= BIT0; s += primeX2 + 1;
                   while (s < loopLimit) {
                     *s               &= BIT5; s += primeX6;
                      s[5]            &= BIT1;
                      s[primeX4 +  8] &= BIT2; s += primeX6;
                      s[9]            &= BIT6;
                      s[primeX4 + 12] &= BIT7; s += primeX6;
                      s[14]           &= BIT3;
                      s[primeX4 + 17] &= BIT4; s += primeX6;
                      s[primeX4 + 22] &= BIT0; s += primeX6 + 23;
                   }
                   if (s >= sieveEnd) { wPrime->setWheelIndex(41); break; }
        }
        break;
        // for sieving primes of type i * 30 + 29
        for (;;) {
          case 49: *s &= BIT6; s += primeX6 + 6; if (s >= sieveEnd) { wPrime->setWheelIndex(50); break; }
          case 50: *s &= BIT5; s += primeX4 + 4; if (s >= sieveEnd) { wPrime->setWheelIndex(51); break; }
          case 51: *s &= BIT4; s += primeX2 + 2; if (s >= sieveEnd) { wPrime->setWheelIndex(52); break; }
          case 52: *s &= BIT3; s += primeX4 + 4; if (s >= sieveEnd) { wPrime->setWheelIndex(53); break; }
          case 53: *s &= BIT2; s += primeX2 + 2; if (s >= sieveEnd) { wPrime->setWheelIndex(54); break; }
          case 54: *s &= BIT1; s += primeX4 + 4; if (s >= sieveEnd) { wPrime->setWheelIndex(55); break; }
          case 55: *s &= BIT0; s += primeX6 + 5; if (s >= sieveEnd) { wPrime->setWheelIndex(48); break; }
          case 48: *s &= BIT7; s += primeX2 + 2;
                   while (s < loopLimit) {
                     *s               &= BIT6; s += primeX6;
                      s[6]            &= BIT5;
                      s[primeX4 + 10] &= BIT4; s += primeX6;
                      s[12]           &= BIT3;
                      s[primeX4 + 16] &= BIT2; s += primeX6;
                      s[18]           &= BIT1;
                      s[primeX4 + 22] &= BIT0; s += primeX6;
                      s[primeX4 + 27] &= BIT7; s += primeX6 + 29;
                   }
                   if (s >= sieveEnd) { wPrime->setWheelIndex(49); break; }
        }
        break;
        // for sieving primes of type i * 30 + 31
        for (;;) {
          case 57: *s &= BIT7; s += primeX6 + 1; if (s >= sieveEnd) { wPrime->setWheelIndex(58); break; }
          case 58: *s &= BIT0; s += primeX4 + 0; if (s >= sieveEnd) { wPrime->setWheelIndex(59); break; }
          case 59: *s &= BIT1; s += primeX2 + 0; if (s >= sieveEnd) { wPrime->setWheelIndex(60); break; }
          case 60: *s &= BIT2; s += primeX4 + 0; if (s >= sieveEnd) { wPrime->setWheelIndex(61); break; }
          case 61: *s &= BIT3; s += primeX2 + 0; if (s >= sieveEnd) { wPrime->setWheelIndex(62); break; }
          case 62: *s &= BIT4; s += primeX4 + 0; if (s >= sieveEnd) { wPrime->setWheelIndex(63); break; }
          case 63: *s &= BIT5; s += primeX6 + 0; if (s >= sieveEnd) { wPrime->setWheelIndex(56); break; }
          case 56: *s &= BIT6; s += primeX2 + 0;
                   while (s < loopLimit) {
                     *s          &= BIT7; s += primeX6 + 1;
                     *s          &= BIT0;
                      s[primeX4] &= BIT1; s += primeX6;
                     *s          &= BIT2;
                      s[primeX4] &= BIT3; s += primeX6;
                     *s          &= BIT4;
                      s[primeX4] &= BIT5; s += primeX6;
                      s[primeX4] &= BIT6; s += primeX6;
                   }
                   if (s >= sieveEnd) { wPrime->setWheelIndex(57); break; }
        }
        break;
      }
      // set the next multiple's index for the next segment
      wPrime->setMultipleIndex(static_cast<uint32_t> (s - sieveEnd));
    }
  }
}
