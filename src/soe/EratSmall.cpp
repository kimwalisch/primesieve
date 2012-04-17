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
#include "EratBase.h"
#include "WheelFactorization.h"
#include "config.h"
#include "bits.h"

#include <stdint.h>
#include <algorithm>
#include <stdexcept>
#include <list>

namespace soe {

EratSmall::EratSmall(const SieveOfEratosthenes& soe) :
  EratBase<Modulo30Wheel_t>(soe)
{
  uint32_t sqrtStop = soe.getSquareRoot();
  uint32_t max = static_cast<uint32_t>(soe.getSieveSize() * config::FACTOR_ERATSMALL);
  uint32_t limit = std::min(sqrtStop, max);
  // sieveSize - 1 + (sievingPrime / 30) * 6 + 6 - sieveSize < sieveSize
  // prevents segmentation faults in sieve(uint8_t*, uint32_t)
  if (limit >= (soe.getSieveSize() - 5) * 5)
    throw std::invalid_argument("EratSmall: limit must be < (sieveSize - 5) * 5.");
  this->setLimit(limit);
}

/**
 * Implementation of the segmented sieve of Eratosthenes with wheel
 * factorization optimized for small sieving primes that have many
 * multiples per segment.
 * @see SieveOfEratosthenes::crossOffMultiples()
 */
void EratSmall::sieve(uint8_t* sieve, uint32_t sieveSize) {
  uint8_t* const sieveEnd = &sieve[sieveSize];

  for (BucketList_t::iterator bucket = buckets_.begin(); bucket != buckets_.end(); ++bucket) {
    WheelPrime* wPrime    = bucket->begin();
    WheelPrime* const end = bucket->end();
    // process the sieving primes within the current bucket
    for (; wPrime != end; wPrime++) {
      const uint32_t sievingPrime  = wPrime->getSievingPrime();
      const uint32_t multipleIndex = wPrime->getMultipleIndex();
      const uint32_t wheelIndex    = wPrime->getWheelIndex();
      const uint32_t maxLoopOffset = sievingPrime * 30 + 29;

      uint8_t* const loopLimit = (maxLoopOffset < sieveSize) ? sieveEnd - maxLoopOffset : sieve;
      uint8_t* s0 = &sieve[multipleIndex];
      uint8_t* s1;

      // cross-off the multiples (unset corresponding bits) of the
      // current sievingPrime within the sieve array using a hardcoded
      // modulo 30 wheel that skips multiples of 2, 3 and 5
      switch (wheelIndex) {
        // for sieving primes of type i * 30 + 7
        for (;;) {
          case 0: *s0 &= BIT0; s0 += sievingPrime * 6 + 1; if (s0 >= sieveEnd) { wPrime->setWheelIndex(1); break; }
          case 1: *s0 &= BIT4; s0 += sievingPrime * 4 + 1; if (s0 >= sieveEnd) { wPrime->setWheelIndex(2); break; }
          case 2: *s0 &= BIT3; s0 += sievingPrime * 2 + 0; if (s0 >= sieveEnd) { wPrime->setWheelIndex(3); break; }
          case 3: *s0 &= BIT7; s0 += sievingPrime * 4 + 1; if (s0 >= sieveEnd) { wPrime->setWheelIndex(4); break; }
          case 4: *s0 &= BIT6; s0 += sievingPrime * 2 + 1; if (s0 >= sieveEnd) { wPrime->setWheelIndex(5); break; }
          case 5: *s0 &= BIT2; s0 += sievingPrime * 4 + 1; if (s0 >= sieveEnd) { wPrime->setWheelIndex(6); break; }
          case 6: *s0 &= BIT1; s0 += sievingPrime * 6 + 1; if (s0 >= sieveEnd) { wPrime->setWheelIndex(7); break; }
          case 7: *s0 &= BIT5; s0 += sievingPrime * 2 + 1;
                  // fast loop, takes only =~ 16 asm instructions per loop
                  // iteration to cross-off the next 8 multiples.
                  // two pointers (s0, s1) are used to break the dependency chain
                  // and take advantage of Instruction-Level Parallelism.
                  s1 = s0 + sievingPrime * 6;
                  while (s0 < loopLimit) {
                    s0[0] &= BIT0; s0 += sievingPrime * 10;
                    s1[1] &= BIT4; s1 += sievingPrime * 6;
                    s0[2] &= BIT3; s0 += sievingPrime * 6;
                    s1[2] &= BIT7; s1 += sievingPrime * 6;
                    s0[3] &= BIT6; s0 += sievingPrime * 6;
                    s1[4] &= BIT2; s1 += sievingPrime * 10;
                    s0[5] &= BIT1; s0 += sievingPrime * 8 + 7;
                    s1[6] &= BIT5; s1 += sievingPrime * 8 + 7;
                  }
                  if (s0 >= sieveEnd) { wPrime->setWheelIndex(0); break; }
        }
        break;
        // for sieving primes of type i * 30 + 11
        for (;;) {
          case  8: *s0 &= BIT1; s0 += sievingPrime * 6 + 2; if (s0 >= sieveEnd) { wPrime->setWheelIndex(9);  break; }
          case  9: *s0 &= BIT3; s0 += sievingPrime * 4 + 1; if (s0 >= sieveEnd) { wPrime->setWheelIndex(10); break; }
          case 10: *s0 &= BIT7; s0 += sievingPrime * 2 + 1; if (s0 >= sieveEnd) { wPrime->setWheelIndex(11); break; }
          case 11: *s0 &= BIT5; s0 += sievingPrime * 4 + 2; if (s0 >= sieveEnd) { wPrime->setWheelIndex(12); break; }
          case 12: *s0 &= BIT0; s0 += sievingPrime * 2 + 0; if (s0 >= sieveEnd) { wPrime->setWheelIndex(13); break; }
          case 13: *s0 &= BIT6; s0 += sievingPrime * 4 + 2; if (s0 >= sieveEnd) { wPrime->setWheelIndex(14); break; }
          case 14: *s0 &= BIT2; s0 += sievingPrime * 6 + 2; if (s0 >= sieveEnd) { wPrime->setWheelIndex(15); break; }
          case 15: *s0 &= BIT4; s0 += sievingPrime * 2 + 1;
                   s1 = s0 + sievingPrime * 6;
                   while (s0 < loopLimit) {
                     s0[0]  &= BIT1; s0 += sievingPrime * 10;
                     s1[2]  &= BIT3; s1 += sievingPrime * 6;
                     s0[3]  &= BIT7; s0 += sievingPrime * 6;
                     s1[4]  &= BIT5; s1 += sievingPrime * 6;
                     s0[6]  &= BIT0; s0 += sievingPrime * 6;
                     s1[6]  &= BIT6; s1 += sievingPrime * 10;
                     s0[8]  &= BIT2; s0 += sievingPrime * 8 + 11;
                     s1[10] &= BIT4; s1 += sievingPrime * 8 + 11;
                   }
                   if (s0 >= sieveEnd) { wPrime->setWheelIndex(8); break; }
        }
        break;
        // for sieving primes of type i * 30 + 13
        for (;;) {
          case 16: *s0 &= BIT2; s0 += sievingPrime * 6 + 2; if (s0 >= sieveEnd) { wPrime->setWheelIndex(17); break; }
          case 17: *s0 &= BIT7; s0 += sievingPrime * 4 + 2; if (s0 >= sieveEnd) { wPrime->setWheelIndex(18); break; }
          case 18: *s0 &= BIT5; s0 += sievingPrime * 2 + 1; if (s0 >= sieveEnd) { wPrime->setWheelIndex(19); break; }
          case 19: *s0 &= BIT4; s0 += sievingPrime * 4 + 2; if (s0 >= sieveEnd) { wPrime->setWheelIndex(20); break; }
          case 20: *s0 &= BIT1; s0 += sievingPrime * 2 + 1; if (s0 >= sieveEnd) { wPrime->setWheelIndex(21); break; }
          case 21: *s0 &= BIT0; s0 += sievingPrime * 4 + 1; if (s0 >= sieveEnd) { wPrime->setWheelIndex(22); break; }
          case 22: *s0 &= BIT6; s0 += sievingPrime * 6 + 3; if (s0 >= sieveEnd) { wPrime->setWheelIndex(23); break; }
          case 23: *s0 &= BIT3; s0 += sievingPrime * 2 + 1;
                   s1 = s0 + sievingPrime * 6;
                   while (s0 < loopLimit) {
                     s0[0]  &= BIT2; s0 += sievingPrime * 10;
                     s1[2]  &= BIT7; s1 += sievingPrime * 6;
                     s0[4]  &= BIT5; s0 += sievingPrime * 6;
                     s1[5]  &= BIT4; s1 += sievingPrime * 6;
                     s0[7]  &= BIT1; s0 += sievingPrime * 6;
                     s1[8]  &= BIT0; s1 += sievingPrime * 10;
                     s0[9]  &= BIT6; s0 += sievingPrime * 8 + 13;
                     s1[12] &= BIT3; s1 += sievingPrime * 8 + 13;
                   }
                   if (s0 >= sieveEnd) { wPrime->setWheelIndex(16); break; }
        }
        break;
        // for sieving primes of type i * 30 + 17
        for (;;) {
          case 24: *s0 &= BIT3; s0 += sievingPrime * 6 + 3; if (s0 >= sieveEnd) { wPrime->setWheelIndex(25); break; }
          case 25: *s0 &= BIT6; s0 += sievingPrime * 4 + 3; if (s0 >= sieveEnd) { wPrime->setWheelIndex(26); break; }
          case 26: *s0 &= BIT0; s0 += sievingPrime * 2 + 1; if (s0 >= sieveEnd) { wPrime->setWheelIndex(27); break; }
          case 27: *s0 &= BIT1; s0 += sievingPrime * 4 + 2; if (s0 >= sieveEnd) { wPrime->setWheelIndex(28); break; }
          case 28: *s0 &= BIT4; s0 += sievingPrime * 2 + 1; if (s0 >= sieveEnd) { wPrime->setWheelIndex(29); break; }
          case 29: *s0 &= BIT5; s0 += sievingPrime * 4 + 2; if (s0 >= sieveEnd) { wPrime->setWheelIndex(30); break; }
          case 30: *s0 &= BIT7; s0 += sievingPrime * 6 + 4; if (s0 >= sieveEnd) { wPrime->setWheelIndex(31); break; }
          case 31: *s0 &= BIT2; s0 += sievingPrime * 2 + 1;
                   s1 = s0 + sievingPrime * 6;
                   while (s0 < loopLimit) {
                     s0[0]  &= BIT3; s0 += sievingPrime * 10;
                     s1[3]  &= BIT6; s1 += sievingPrime * 6;
                     s0[6]  &= BIT0; s0 += sievingPrime * 6;
                     s1[7]  &= BIT1; s1 += sievingPrime * 6;
                     s0[9]  &= BIT4; s0 += sievingPrime * 6;
                     s1[10] &= BIT5; s1 += sievingPrime * 10;
                     s0[12] &= BIT7; s0 += sievingPrime * 8 + 17;
                     s1[16] &= BIT2; s1 += sievingPrime * 8 + 17;
                   }
                   if (s0 >= sieveEnd) { wPrime->setWheelIndex(24); break; }
        }
        break;
        // for sieving primes of type i * 30 + 19
        for (;;) {
          case 32: *s0 &= BIT4; s0 += sievingPrime * 6 + 4; if (s0 >= sieveEnd) { wPrime->setWheelIndex(33); break; }
          case 33: *s0 &= BIT2; s0 += sievingPrime * 4 + 2; if (s0 >= sieveEnd) { wPrime->setWheelIndex(34); break; }
          case 34: *s0 &= BIT6; s0 += sievingPrime * 2 + 2; if (s0 >= sieveEnd) { wPrime->setWheelIndex(35); break; }
          case 35: *s0 &= BIT0; s0 += sievingPrime * 4 + 2; if (s0 >= sieveEnd) { wPrime->setWheelIndex(36); break; }
          case 36: *s0 &= BIT5; s0 += sievingPrime * 2 + 1; if (s0 >= sieveEnd) { wPrime->setWheelIndex(37); break; }
          case 37: *s0 &= BIT7; s0 += sievingPrime * 4 + 3; if (s0 >= sieveEnd) { wPrime->setWheelIndex(38); break; }
          case 38: *s0 &= BIT3; s0 += sievingPrime * 6 + 4; if (s0 >= sieveEnd) { wPrime->setWheelIndex(39); break; }
          case 39: *s0 &= BIT1; s0 += sievingPrime * 2 + 1;
                   s1 = s0 + sievingPrime * 6;
                   while (s0 < loopLimit) {
                     s0[0]  &= BIT4; s0 += sievingPrime * 10;
                     s1[4]  &= BIT2; s1 += sievingPrime * 6;
                     s0[6]  &= BIT6; s0 += sievingPrime * 6;
                     s1[8]  &= BIT0; s1 += sievingPrime * 6;
                     s0[10] &= BIT5; s0 += sievingPrime * 6;
                     s1[11] &= BIT7; s1 += sievingPrime * 10;
                     s0[14] &= BIT3; s0 += sievingPrime * 8 + 19;
                     s1[18] &= BIT1; s1 += sievingPrime * 8 + 19;
                   }
                   if (s0 >= sieveEnd) { wPrime->setWheelIndex(32); break; }
        }
        break;
        // for sieving primes of type i * 30 + 23
        for (;;) {
          case 40: *s0 &= BIT5; s0 += sievingPrime * 6 + 5; if (s0 >= sieveEnd) { wPrime->setWheelIndex(41); break; }
          case 41: *s0 &= BIT1; s0 += sievingPrime * 4 + 3; if (s0 >= sieveEnd) { wPrime->setWheelIndex(42); break; }
          case 42: *s0 &= BIT2; s0 += sievingPrime * 2 + 1; if (s0 >= sieveEnd) { wPrime->setWheelIndex(43); break; }
          case 43: *s0 &= BIT6; s0 += sievingPrime * 4 + 3; if (s0 >= sieveEnd) { wPrime->setWheelIndex(44); break; }
          case 44: *s0 &= BIT7; s0 += sievingPrime * 2 + 2; if (s0 >= sieveEnd) { wPrime->setWheelIndex(45); break; }
          case 45: *s0 &= BIT3; s0 += sievingPrime * 4 + 3; if (s0 >= sieveEnd) { wPrime->setWheelIndex(46); break; }
          case 46: *s0 &= BIT4; s0 += sievingPrime * 6 + 5; if (s0 >= sieveEnd) { wPrime->setWheelIndex(47); break; }
          case 47: *s0 &= BIT0; s0 += sievingPrime * 2 + 1;
                   s1 = s0 + sievingPrime * 6;
                   while (s0 < loopLimit) {
                     s0[0]  &= BIT5; s0 += sievingPrime * 10;
                     s1[5]  &= BIT1; s1 += sievingPrime * 6;
                     s0[8]  &= BIT2; s0 += sievingPrime * 6;
                     s1[9]  &= BIT6; s1 += sievingPrime * 6;
                     s0[12] &= BIT7; s0 += sievingPrime * 6;
                     s1[14] &= BIT3; s1 += sievingPrime * 10;
                     s0[17] &= BIT4; s0 += sievingPrime * 8 + 23;
                     s1[22] &= BIT0; s1 += sievingPrime * 8 + 23;
                   }
                   if (s0 >= sieveEnd) { wPrime->setWheelIndex(40); break; }
        }
        break;
        // for sieving primes of type i * 30 + 29
        for (;;) {
          case 48: *s0 &= BIT6; s0 += sievingPrime * 6 + 6; if (s0 >= sieveEnd) { wPrime->setWheelIndex(49); break; }
          case 49: *s0 &= BIT5; s0 += sievingPrime * 4 + 4; if (s0 >= sieveEnd) { wPrime->setWheelIndex(50); break; }
          case 50: *s0 &= BIT4; s0 += sievingPrime * 2 + 2; if (s0 >= sieveEnd) { wPrime->setWheelIndex(51); break; }
          case 51: *s0 &= BIT3; s0 += sievingPrime * 4 + 4; if (s0 >= sieveEnd) { wPrime->setWheelIndex(52); break; }
          case 52: *s0 &= BIT2; s0 += sievingPrime * 2 + 2; if (s0 >= sieveEnd) { wPrime->setWheelIndex(53); break; }
          case 53: *s0 &= BIT1; s0 += sievingPrime * 4 + 4; if (s0 >= sieveEnd) { wPrime->setWheelIndex(54); break; }
          case 54: *s0 &= BIT0; s0 += sievingPrime * 6 + 5; if (s0 >= sieveEnd) { wPrime->setWheelIndex(55); break; }
          case 55: *s0 &= BIT7; s0 += sievingPrime * 2 + 2;
                   s1 = s0 + sievingPrime * 6;
                   while (s0 < loopLimit) {
                     s0[0]  &= BIT6; s0 += sievingPrime * 10;
                     s1[6]  &= BIT5; s1 += sievingPrime * 6;
                     s0[10] &= BIT4; s0 += sievingPrime * 6;
                     s1[12] &= BIT3; s1 += sievingPrime * 6;
                     s0[16] &= BIT2; s0 += sievingPrime * 6;
                     s1[18] &= BIT1; s1 += sievingPrime * 10;
                     s0[22] &= BIT0; s0 += sievingPrime * 8 + 29;
                     s1[27] &= BIT7; s1 += sievingPrime * 8 + 29;
                   }
                   if (s0 >= sieveEnd) { wPrime->setWheelIndex(48); break; }
        }
        break;
        // for sieving primes of type i * 30 + 31
        for (;;) {
          case 56: *s0 &= BIT7; s0 += sievingPrime * 6 + 1; if (s0 >= sieveEnd) { wPrime->setWheelIndex(57); break; }
          case 57: *s0 &= BIT0; s0 += sievingPrime * 4 + 0; if (s0 >= sieveEnd) { wPrime->setWheelIndex(58); break; }
          case 58: *s0 &= BIT1; s0 += sievingPrime * 2 + 0; if (s0 >= sieveEnd) { wPrime->setWheelIndex(59); break; }
          case 59: *s0 &= BIT2; s0 += sievingPrime * 4 + 0; if (s0 >= sieveEnd) { wPrime->setWheelIndex(60); break; }
          case 60: *s0 &= BIT3; s0 += sievingPrime * 2 + 0; if (s0 >= sieveEnd) { wPrime->setWheelIndex(61); break; }
          case 61: *s0 &= BIT4; s0 += sievingPrime * 4 + 0; if (s0 >= sieveEnd) { wPrime->setWheelIndex(62); break; }
          case 62: *s0 &= BIT5; s0 += sievingPrime * 6 + 0; if (s0 >= sieveEnd) { wPrime->setWheelIndex(63); break; }
          case 63: *s0 &= BIT6; s0 += sievingPrime * 2 + 0;
                   s1 = s0 + sievingPrime * 6;
                   while (s0 < loopLimit) {
                     s0[0] &= BIT7; s0 += sievingPrime * 10;
                     s1[1] &= BIT0; s1 += sievingPrime * 6;
                     s0[1] &= BIT1; s0 += sievingPrime * 6;
                     s1[1] &= BIT2; s1 += sievingPrime * 6;
                     s0[1] &= BIT3; s0 += sievingPrime * 6;
                     s1[1] &= BIT4; s1 += sievingPrime * 10;
                     s0[1] &= BIT5; s0 += sievingPrime * 8 + 1;
                     s1[1] &= BIT6; s1 += sievingPrime * 8 + 1;
                   }
                   if (s0 >= sieveEnd) { wPrime->setWheelIndex(56); break; }
        }
        break;
      }
      // set the next multiple's index for the next segment
      wPrime->setMultipleIndex(static_cast<uint32_t>(s0 - sieveEnd));
    }
  }
}

} // namespace soe
