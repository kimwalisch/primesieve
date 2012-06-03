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
  uint_t sqrtStop = soe.getSquareRoot();
  uint_t max = static_cast<uint_t>(soe.getSieveSize() * config::FACTOR_ERATSMALL);
  uint_t limit = std::min(sqrtStop, max);
  // sieveSize - 1 + (sievingPrime / 30) * 6 + 6 - sieveSize < sieveSize
  // prevents segmentation faults in sieve(uint8_t*, uint_t)
  if (limit >= (soe.getSieveSize() - 5) * 5)
    throw std::invalid_argument("EratSmall: limit must be < (sieveSize - 5) * 5.");
  this->setLimit(limit);
}

/// Implementation of the segmented sieve of Eratosthenes with
/// wheel factorization optimized for small sieving primes that have
/// many multiples per segment.
/// @see SieveOfEratosthenes::crossOffMultiples()
///
void EratSmall::crossOff(uint8_t* sieve, uint_t sieveSize) {
  uint8_t* const sieveEnd = &sieve[sieveSize];

  for (BucketList_t::iterator bucket = buckets_.begin(); bucket != buckets_.end(); ++bucket) {
    WheelPrime* wPrime    = bucket->begin();
    WheelPrime* const end = bucket->end();

    for (; wPrime != end; wPrime++) {
      const uint_t sievingPrime  = wPrime->getSievingPrime();
      const uint_t multipleIndex = wPrime->getMultipleIndex();
      const uint_t wheelIndex    = wPrime->getWheelIndex();

      const uint_t loopDistance = sievingPrime * 30 + 29;
      uint8_t* const loopLimit  = (loopDistance < sieveSize) ? sieveEnd - loopDistance : sieve;

      // pointer to the byte containing the first multiple of
      // sievingPrime within the current segment
      uint8_t* p = &sieve[multipleIndex];
      uint8_t* q;

      // cross-off the multiples (unset bits) of the current sievingPrime
      // within the sieve array using a hardcoded modulo 30 wheel that
      // skips multiples of 2, 3 and 5
      switch (wheelIndex) {
        // for sieving primes of type i*30 + 7
        for (;;) {
          case 0: // fast loop, takes only =~ 16 asm instructions per
                  // iteration to cross-off the next 8 multiples. Two
                  // pointers (p, q) are used to break the dependency chain
                  // and take advantage of Instruction-Level Parallelism.
                  while (p < loopLimit) {
                    q = p + sievingPrime * 6;
                    p[0] &= BIT0; p += sievingPrime * 10;
                    q[1] &= BIT4; q += sievingPrime * 6;
                    p[2] &= BIT3; p += sievingPrime * 6;
                    q[2] &= BIT7; q += sievingPrime * 6;
                    p[3] &= BIT6; p += sievingPrime * 6;
                    q[4] &= BIT2; q += sievingPrime * 10;
                    p[5] &= BIT1; p += sievingPrime * 8 + 7;
                    q[6] &= BIT5;
                  }
                                                         if (p >= sieveEnd) { wPrime->setWheelIndex(0); break; }
                  *p &= BIT0; p += sievingPrime * 6 + 1; if (p >= sieveEnd) { wPrime->setWheelIndex(1); break; }
          case 1: *p &= BIT4; p += sievingPrime * 4 + 1; if (p >= sieveEnd) { wPrime->setWheelIndex(2); break; }
          case 2: *p &= BIT3; p += sievingPrime * 2 + 0; if (p >= sieveEnd) { wPrime->setWheelIndex(3); break; }
          case 3: *p &= BIT7; p += sievingPrime * 4 + 1; if (p >= sieveEnd) { wPrime->setWheelIndex(4); break; }
          case 4: *p &= BIT6; p += sievingPrime * 2 + 1; if (p >= sieveEnd) { wPrime->setWheelIndex(5); break; }
          case 5: *p &= BIT2; p += sievingPrime * 4 + 1; if (p >= sieveEnd) { wPrime->setWheelIndex(6); break; }
          case 6: *p &= BIT1; p += sievingPrime * 6 + 1; if (p >= sieveEnd) { wPrime->setWheelIndex(7); break; }
          case 7: *p &= BIT5; p += sievingPrime * 2 + 1;
        }
        break;
        // for sieving primes of type i*30 + 11
        for (;;) {
          case  8: while (p < loopLimit) {
                     q = p + sievingPrime * 6;
                     p[0]  &= BIT1; p += sievingPrime * 10;
                     q[2]  &= BIT3; q += sievingPrime * 6;
                     p[3]  &= BIT7; p += sievingPrime * 6;
                     q[4]  &= BIT5; q += sievingPrime * 6;
                     p[6]  &= BIT0; p += sievingPrime * 6;
                     q[6]  &= BIT6; q += sievingPrime * 10;
                     p[8]  &= BIT2; p += sievingPrime * 8 + 11;
                     q[10] &= BIT4;
                   }
                                                          if (p >= sieveEnd) { wPrime->setWheelIndex(8); break; }
                   *p &= BIT1; p += sievingPrime * 6 + 2; if (p >= sieveEnd) { wPrime->setWheelIndex(9);  break; }
          case  9: *p &= BIT3; p += sievingPrime * 4 + 1; if (p >= sieveEnd) { wPrime->setWheelIndex(10); break; }
          case 10: *p &= BIT7; p += sievingPrime * 2 + 1; if (p >= sieveEnd) { wPrime->setWheelIndex(11); break; }
          case 11: *p &= BIT5; p += sievingPrime * 4 + 2; if (p >= sieveEnd) { wPrime->setWheelIndex(12); break; }
          case 12: *p &= BIT0; p += sievingPrime * 2 + 0; if (p >= sieveEnd) { wPrime->setWheelIndex(13); break; }
          case 13: *p &= BIT6; p += sievingPrime * 4 + 2; if (p >= sieveEnd) { wPrime->setWheelIndex(14); break; }
          case 14: *p &= BIT2; p += sievingPrime * 6 + 2; if (p >= sieveEnd) { wPrime->setWheelIndex(15); break; }
          case 15: *p &= BIT4; p += sievingPrime * 2 + 1;
        }
        break;
        // for sieving primes of type i*30 + 13
        for (;;) {
          case 16: while (p < loopLimit) {
                     q = p + sievingPrime * 6;
                     p[0]  &= BIT2; p += sievingPrime * 10;
                     q[2]  &= BIT7; q += sievingPrime * 6;
                     p[4]  &= BIT5; p += sievingPrime * 6;
                     q[5]  &= BIT4; q += sievingPrime * 6;
                     p[7]  &= BIT1; p += sievingPrime * 6;
                     q[8]  &= BIT0; q += sievingPrime * 10;
                     p[9]  &= BIT6; p += sievingPrime * 8 + 13;
                     q[12] &= BIT3;
                   }
                                                          if (p >= sieveEnd) { wPrime->setWheelIndex(16); break; }
                   *p &= BIT2; p += sievingPrime * 6 + 2; if (p >= sieveEnd) { wPrime->setWheelIndex(17); break; }
          case 17: *p &= BIT7; p += sievingPrime * 4 + 2; if (p >= sieveEnd) { wPrime->setWheelIndex(18); break; }
          case 18: *p &= BIT5; p += sievingPrime * 2 + 1; if (p >= sieveEnd) { wPrime->setWheelIndex(19); break; }
          case 19: *p &= BIT4; p += sievingPrime * 4 + 2; if (p >= sieveEnd) { wPrime->setWheelIndex(20); break; }
          case 20: *p &= BIT1; p += sievingPrime * 2 + 1; if (p >= sieveEnd) { wPrime->setWheelIndex(21); break; }
          case 21: *p &= BIT0; p += sievingPrime * 4 + 1; if (p >= sieveEnd) { wPrime->setWheelIndex(22); break; }
          case 22: *p &= BIT6; p += sievingPrime * 6 + 3; if (p >= sieveEnd) { wPrime->setWheelIndex(23); break; }
          case 23: *p &= BIT3; p += sievingPrime * 2 + 1;
        }
        break;
        // for sieving primes of type i*30 + 17
        for (;;) {
          case 24: while (p < loopLimit) {
                     q = p + sievingPrime * 6;
                     p[0]  &= BIT3; p += sievingPrime * 10;
                     q[3]  &= BIT6; q += sievingPrime * 6;
                     p[6]  &= BIT0; p += sievingPrime * 6;
                     q[7]  &= BIT1; q += sievingPrime * 6;
                     p[9]  &= BIT4; p += sievingPrime * 6;
                     q[10] &= BIT5; q += sievingPrime * 10;
                     p[12] &= BIT7; p += sievingPrime * 8 + 17;
                     q[16] &= BIT2;
                   }
                                                          if (p >= sieveEnd) { wPrime->setWheelIndex(24); break; }
                   *p &= BIT3; p += sievingPrime * 6 + 3; if (p >= sieveEnd) { wPrime->setWheelIndex(25); break; }
          case 25: *p &= BIT6; p += sievingPrime * 4 + 3; if (p >= sieveEnd) { wPrime->setWheelIndex(26); break; }
          case 26: *p &= BIT0; p += sievingPrime * 2 + 1; if (p >= sieveEnd) { wPrime->setWheelIndex(27); break; }
          case 27: *p &= BIT1; p += sievingPrime * 4 + 2; if (p >= sieveEnd) { wPrime->setWheelIndex(28); break; }
          case 28: *p &= BIT4; p += sievingPrime * 2 + 1; if (p >= sieveEnd) { wPrime->setWheelIndex(29); break; }
          case 29: *p &= BIT5; p += sievingPrime * 4 + 2; if (p >= sieveEnd) { wPrime->setWheelIndex(30); break; }
          case 30: *p &= BIT7; p += sievingPrime * 6 + 4; if (p >= sieveEnd) { wPrime->setWheelIndex(31); break; }
          case 31: *p &= BIT2; p += sievingPrime * 2 + 1;
        }
        break;
        // for sieving primes of type i*30 + 19
        for (;;) {
          case 32: while (p < loopLimit) {
                     q = p + sievingPrime * 6;
                     p[0]  &= BIT4; p += sievingPrime * 10;
                     q[4]  &= BIT2; q += sievingPrime * 6;
                     p[6]  &= BIT6; p += sievingPrime * 6;
                     q[8]  &= BIT0; q += sievingPrime * 6;
                     p[10] &= BIT5; p += sievingPrime * 6;
                     q[11] &= BIT7; q += sievingPrime * 10;
                     p[14] &= BIT3; p += sievingPrime * 8 + 19;
                     q[18] &= BIT1;
                   }
                                                          if (p >= sieveEnd) { wPrime->setWheelIndex(32); break; }
                   *p &= BIT4; p += sievingPrime * 6 + 4; if (p >= sieveEnd) { wPrime->setWheelIndex(33); break; }
          case 33: *p &= BIT2; p += sievingPrime * 4 + 2; if (p >= sieveEnd) { wPrime->setWheelIndex(34); break; }
          case 34: *p &= BIT6; p += sievingPrime * 2 + 2; if (p >= sieveEnd) { wPrime->setWheelIndex(35); break; }
          case 35: *p &= BIT0; p += sievingPrime * 4 + 2; if (p >= sieveEnd) { wPrime->setWheelIndex(36); break; }
          case 36: *p &= BIT5; p += sievingPrime * 2 + 1; if (p >= sieveEnd) { wPrime->setWheelIndex(37); break; }
          case 37: *p &= BIT7; p += sievingPrime * 4 + 3; if (p >= sieveEnd) { wPrime->setWheelIndex(38); break; }
          case 38: *p &= BIT3; p += sievingPrime * 6 + 4; if (p >= sieveEnd) { wPrime->setWheelIndex(39); break; }
          case 39: *p &= BIT1; p += sievingPrime * 2 + 1;
        }
        break;
        // for sieving primes of type i*30 + 23
        for (;;) {
          case 40: while (p < loopLimit) {
                     q = p + sievingPrime * 6;
                     p[0]  &= BIT5; p += sievingPrime * 10;
                     q[5]  &= BIT1; q += sievingPrime * 6;
                     p[8]  &= BIT2; p += sievingPrime * 6;
                     q[9]  &= BIT6; q += sievingPrime * 6;
                     p[12] &= BIT7; p += sievingPrime * 6;
                     q[14] &= BIT3; q += sievingPrime * 10;
                     p[17] &= BIT4; p += sievingPrime * 8 + 23;
                     q[22] &= BIT0;
                   }
                                                          if (p >= sieveEnd) { wPrime->setWheelIndex(40); break; }
                   *p &= BIT5; p += sievingPrime * 6 + 5; if (p >= sieveEnd) { wPrime->setWheelIndex(41); break; }
          case 41: *p &= BIT1; p += sievingPrime * 4 + 3; if (p >= sieveEnd) { wPrime->setWheelIndex(42); break; }
          case 42: *p &= BIT2; p += sievingPrime * 2 + 1; if (p >= sieveEnd) { wPrime->setWheelIndex(43); break; }
          case 43: *p &= BIT6; p += sievingPrime * 4 + 3; if (p >= sieveEnd) { wPrime->setWheelIndex(44); break; }
          case 44: *p &= BIT7; p += sievingPrime * 2 + 2; if (p >= sieveEnd) { wPrime->setWheelIndex(45); break; }
          case 45: *p &= BIT3; p += sievingPrime * 4 + 3; if (p >= sieveEnd) { wPrime->setWheelIndex(46); break; }
          case 46: *p &= BIT4; p += sievingPrime * 6 + 5; if (p >= sieveEnd) { wPrime->setWheelIndex(47); break; }
          case 47: *p &= BIT0; p += sievingPrime * 2 + 1;
        }
        break;
        // for sieving primes of type i*30 + 29
        for (;;) {
          case 48: while (p < loopLimit) {
                     q = p + sievingPrime * 6;
                     p[0]  &= BIT6; p += sievingPrime * 10;
                     q[6]  &= BIT5; q += sievingPrime * 6;
                     p[10] &= BIT4; p += sievingPrime * 6;
                     q[12] &= BIT3; q += sievingPrime * 6;
                     p[16] &= BIT2; p += sievingPrime * 6;
                     q[18] &= BIT1; q += sievingPrime * 10;
                     p[22] &= BIT0; p += sievingPrime * 8 + 29;
                     q[27] &= BIT7;
                   }
                                                          if (p >= sieveEnd) { wPrime->setWheelIndex(48); break; }
                   *p &= BIT6; p += sievingPrime * 6 + 6; if (p >= sieveEnd) { wPrime->setWheelIndex(49); break; }
          case 49: *p &= BIT5; p += sievingPrime * 4 + 4; if (p >= sieveEnd) { wPrime->setWheelIndex(50); break; }
          case 50: *p &= BIT4; p += sievingPrime * 2 + 2; if (p >= sieveEnd) { wPrime->setWheelIndex(51); break; }
          case 51: *p &= BIT3; p += sievingPrime * 4 + 4; if (p >= sieveEnd) { wPrime->setWheelIndex(52); break; }
          case 52: *p &= BIT2; p += sievingPrime * 2 + 2; if (p >= sieveEnd) { wPrime->setWheelIndex(53); break; }
          case 53: *p &= BIT1; p += sievingPrime * 4 + 4; if (p >= sieveEnd) { wPrime->setWheelIndex(54); break; }
          case 54: *p &= BIT0; p += sievingPrime * 6 + 5; if (p >= sieveEnd) { wPrime->setWheelIndex(55); break; }
          case 55: *p &= BIT7; p += sievingPrime * 2 + 2;
        }
        break;
        // for sieving primes of type i*30 + 31
        for (;;) {
          case 56: while (p < loopLimit) {
                     q = p + sievingPrime * 6;
                     p[0] &= BIT7; p += sievingPrime * 10;
                     q[1] &= BIT0; q += sievingPrime * 6;
                     p[1] &= BIT1; p += sievingPrime * 6;
                     q[1] &= BIT2; q += sievingPrime * 6;
                     p[1] &= BIT3; p += sievingPrime * 6;
                     q[1] &= BIT4; q += sievingPrime * 10;
                     p[1] &= BIT5; p += sievingPrime * 8 + 1;
                     q[1] &= BIT6;
                   }
                                                          if (p >= sieveEnd) { wPrime->setWheelIndex(56); break; }
                   *p &= BIT7; p += sievingPrime * 6 + 1; if (p >= sieveEnd) { wPrime->setWheelIndex(57); break; }
          case 57: *p &= BIT0; p += sievingPrime * 4 + 0; if (p >= sieveEnd) { wPrime->setWheelIndex(58); break; }
          case 58: *p &= BIT1; p += sievingPrime * 2 + 0; if (p >= sieveEnd) { wPrime->setWheelIndex(59); break; }
          case 59: *p &= BIT2; p += sievingPrime * 4 + 0; if (p >= sieveEnd) { wPrime->setWheelIndex(60); break; }
          case 60: *p &= BIT3; p += sievingPrime * 2 + 0; if (p >= sieveEnd) { wPrime->setWheelIndex(61); break; }
          case 61: *p &= BIT4; p += sievingPrime * 4 + 0; if (p >= sieveEnd) { wPrime->setWheelIndex(62); break; }
          case 62: *p &= BIT5; p += sievingPrime * 6 + 0; if (p >= sieveEnd) { wPrime->setWheelIndex(63); break; }
          case 63: *p &= BIT6; p += sievingPrime * 2 + 0;
        }
        break;
      }
      // set multipleIndex for the next segment
      wPrime->setMultipleIndex(static_cast<uint_t>(p - sieveEnd));
    }
  }
}

} // namespace soe
