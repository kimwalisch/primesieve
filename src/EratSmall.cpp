/*
 * EratSmall.cpp -- This file is part of primesieve
 *
 * Copyright (C) 2010 Kim Walisch, <kim.walisch@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include "EratSmall.h"
#include "bits.h"

#include <stdint.h>
#include <stdexcept>
#include <cstdlib>

EratSmall::EratSmall(uint32_t limit, uint64_t stopNumber, uint32_t sieveSize) :
  EratBase<Modulo30Wheel> (limit, stopNumber, sieveSize) {
  // the following equation prevents array segmentation faults in 
  // sieve(uint8_t*, uint32_t) :
  // sieveSize - 1 + (primeNumber / 15) * 3 + 3 - sieveSize < sieveSize
  if (limit_ >= (sieveSize - 2) * 5) {
    throw std::logic_error("EratSmall: limit must be < (sieveSize - 2) * 5.");
  }
}

/**
 * Implementation of the segmented sieve of Eratosthenes with wheel
 * factorization (modulo 30 wheel). Is used to cross-off the 
 * multiples of the current sieve round.
 */
void EratSmall::sieve(uint8_t* sieve, uint32_t sieveSize) {
  uint8_t* const sieveEnd = &sieve[sieveSize];
  
  // iterate over all the buckets of the bucket list
  for (Bucket_t* bucket = bucketList_; bucket != NULL; bucket = bucket->next) {
    // iterate over the wheelPrimes of the current bucket
    WheelPrime* wPrime = bucket->wheelPrimeBegin();
    WheelPrime* end    = bucket->wheelPrimeEnd();
    for (; wPrime != end; wPrime++) {
      const uint32_t primeX2 = wPrime->getSievePrime();
      const uint32_t primeX4 = primeX2 + primeX2;
      const uint32_t primeX6 = primeX2 + primeX4;

      // a pointer here is faster than an index
      uint8_t* s = &sieve[wPrime->getSieveIndex()];
      switch (wPrime->getWheelIndex()) {
        // for sieving primes of type n * 30 + 7
        for (;;) {
          case 1:
          *s &= BIT0;
          s += primeX6 + 1;
          if (s >= sieveEnd)
            goto out1;
          case 2:
          *s &= BIT4;
          s += primeX4 + 1;
          if (s >= sieveEnd)
            goto out2;
          case 3:
          *s &= BIT3;
          s += primeX2;
          if (s >= sieveEnd)
            goto out3;
          case 4:
          *s &= BIT7;
          s += primeX4 + 1;
          if (s >= sieveEnd)
            goto out4;
          case 5:
          *s &= BIT6;
          s += primeX2 + 1;
          if (s >= sieveEnd)
            goto out5;
          case 6:
          *s &= BIT2;
          s += primeX4 + 1;
          if (s >= sieveEnd)
            goto out6;
          case 7:
          *s &= BIT1;
          s += primeX6 + 1;
          if (s >= sieveEnd)
            goto out7;
          case 0:
          *s &= BIT5;
          s += primeX2 + 1;
          // fast while loop (less operations)
          while (s + primeX6 * 5 + 7 < sieveEnd) {
            *s &= BIT0;
            s += primeX6;
            s[1] &= BIT4;
            s[primeX4 + 2] &= BIT3;
            s += primeX6;
            s[2] &= BIT7;
            s[primeX4 + 3] &= BIT6;
            s += primeX6;
            s[4] &= BIT2;
            s[primeX4 + 5] &= BIT1;
            s += primeX6;
            s[primeX4 + 6] &= BIT5;
            s += primeX6 + 7;
          }
          if (s >= sieveEnd)
            goto out0;
        }
        // for sieving primes of type n * 30 + 11
        for (;;) {
          case 9:
          *s &= BIT1;
          s += primeX6 + 2;
          if (s >= sieveEnd)
            goto out1;
          case 10:
          *s &= BIT3;
          s += primeX4 + 1;
          if (s >= sieveEnd)
            goto out2;
          case 11:
          *s &= BIT7;
          s += primeX2 + 1;
          if (s >= sieveEnd)
            goto out3;
          case 12:
          *s &= BIT5;
          s += primeX4 + 2;
          if (s >= sieveEnd)
            goto out4;
          case 13:
          *s &= BIT0;
          s += primeX2;
          if (s >= sieveEnd)
            goto out5;
          case 14:
          *s &= BIT6;
          s += primeX4 + 2;
          if (s >= sieveEnd)
            goto out6;
          case 15:
          *s &= BIT2;
          s += primeX6 + 2;
          if (s >= sieveEnd)
            goto out7;
          case 8:
          *s &= BIT4;
          s += primeX2 + 1;
          // fast while loop (less operations)
          while (s + primeX6 * 5 + 11 < sieveEnd) {
            *s &= BIT1;
            s += primeX6;
            s[2] &= BIT3;
            s[primeX4 + 3] &= BIT7;
            s += primeX6;
            s[4] &= BIT5;
            s[primeX4 + 6] &= BIT0;
            s += primeX6;
            s[6] &= BIT6;
            s[primeX4 + 8] &= BIT2;
            s += primeX6;
            s[primeX4 + 10] &= BIT4;
            s += primeX6 + 11;
          }
          if (s >= sieveEnd)
            goto out0;
        }
        // for sieving primes of type n * 30 + 13
        for (;;) {
          case 17:
          *s &= BIT2;
          s += primeX6 + 2;
          if (s >= sieveEnd)
            goto out1;
          case 18:
          *s &= BIT7;
          s += primeX4 + 2;
          if (s >= sieveEnd)
            goto out2;
          case 19:
          *s &= BIT5;
          s += primeX2 + 1;
          if (s >= sieveEnd)
            goto out3;
          case 20:
          *s &= BIT4;
          s += primeX4 + 2;
          if (s >= sieveEnd)
            goto out4;
          case 21:
          *s &= BIT1;
          s += primeX2 + 1;
          if (s >= sieveEnd)
            goto out5;
          case 22:
          *s &= BIT0;
          s += primeX4 + 1;
          if (s >= sieveEnd)
            goto out6;
          case 23:
          *s &= BIT6;
          s += primeX6 + 3;
          if (s >= sieveEnd)
            goto out7;
          case 16:
          *s &= BIT3;
          s += primeX2 + 1;
          // fast while loop (less operations)
          while (s + primeX6 * 5 + 13 < sieveEnd) {
            *s &= BIT2;
            s += primeX6;
            s[2] &= BIT7;
            s[primeX4 + 4] &= BIT5;
            s += primeX6;
            s[5] &= BIT4;
            s[primeX4 + 7] &= BIT1;
            s += primeX6;
            s[8] &= BIT0;
            s[primeX4 + 9] &= BIT6;
            s += primeX6;
            s[primeX4 + 12] &= BIT3;
            s += primeX6 + 13;
          }
          if (s >= sieveEnd)
            goto out0;
        }
        // for sieving primes of type n * 30 + 17
        for (;;) {
          case 25:
          *s &= BIT3;
          s += primeX6;
          if (s >= sieveEnd)
            goto out1;
          case 26:
          *s &= BIT6;
          s += primeX4 + 1;
          if (s >= sieveEnd)
            goto out2;
          case 27:
          *s &= BIT0;
          s += primeX2;
          if (s >= sieveEnd)
            goto out3;
          case 28:
          *s &= BIT1;
          s += primeX4;
          if (s >= sieveEnd)
            goto out4;
          case 29:
          *s &= BIT4;
          s += primeX2;
          if (s >= sieveEnd)
            goto out5;
          case 30:
          *s &= BIT5;
          s += primeX4;
          if (s >= sieveEnd)
            goto out6;
          case 31:
          *s &= BIT7;
          s += primeX6 + 1;
          if (s >= sieveEnd)
            goto out7;
          case 24:
          *s &= BIT2;
          s += primeX2;
          // fast while loop (less operations)
          while (s + primeX6 * 5 + 2 < sieveEnd) {
            *s &= BIT3;
            s += primeX6;
            *s &= BIT6;
            s[primeX4 + 1] &= BIT0;
            s += primeX6;
            s[1] &= BIT1;
            s[primeX4 + 1] &= BIT4;
            s += primeX6;
            s[1] &= BIT5;
            s[primeX4 + 1] &= BIT7;
            s += primeX6 + 2;
            s[primeX4] &= BIT2;
            s += primeX6;
          }
          if (s >= sieveEnd)
            goto out0;
        }
        // for sieving primes of type n * 30 + 19
        for (;;) {
          case 33:
          *s &= BIT4;
          s += primeX6 + 1;
          if (s >= sieveEnd)
            goto out1;
          case 34:
          *s &= BIT2;
          s += primeX4;
          if (s >= sieveEnd)
            goto out2;
          case 35:
          *s &= BIT6;
          s += primeX2 + 1;
          if (s >= sieveEnd)
            goto out3;
          case 36:
          *s &= BIT0;
          s += primeX4;
          if (s >= sieveEnd)
            goto out4;
          case 37:
          *s &= BIT5;
          s += primeX2;
          if (s >= sieveEnd)
            goto out5;
          case 38:
          *s &= BIT7;
          s += primeX4 + 1;
          if (s >= sieveEnd)
            goto out6;
          case 39:
          *s &= BIT3;
          s += primeX6 + 1;
          if (s >= sieveEnd)
            goto out7;
          case 32:
          *s &= BIT1;
          s += primeX2;
          // fast while loop (less operations)
          while (s + primeX6 * 5 + 4 < sieveEnd) {
            *s &= BIT4;
            s += primeX6;
            s[1] &= BIT2;
            s[primeX4 + 1] &= BIT6;
            s += primeX6;
            s[2] &= BIT0;
            s[primeX4 + 2] &= BIT5;
            s += primeX6;
            s[2] &= BIT7;
            s[primeX4 + 3] &= BIT3;
            s += primeX6 + 4;
            s[primeX4] &= BIT1;
            s += primeX6;
          }
          if (s >= sieveEnd)
            goto out0;
        }
        // for sieving primes of type n * 30 + 23
        for (;;) {
          case 41:
          *s &= BIT5;
          s += primeX6 + 2;
          if (s >= sieveEnd)
            goto out1;
          case 42:
          *s &= BIT1;
          s += primeX4 + 1;
          if (s >= sieveEnd)
            goto out2;
          case 43:
          *s &= BIT2;
          s += primeX2;
          if (s >= sieveEnd)
            goto out3;
          case 44:
          *s &= BIT6;
          s += primeX4 + 1;
          if (s >= sieveEnd)
            goto out4;
          case 45:
          *s &= BIT7;
          s += primeX2 + 1;
          if (s >= sieveEnd)
            goto out5;
          case 46:
          *s &= BIT3;
          s += primeX4 + 1;
          if (s >= sieveEnd)
            goto out6;
          case 47:
          *s &= BIT4;
          s += primeX6 + 2;
          if (s >= sieveEnd)
            goto out7;
          case 40:
          *s &= BIT0;
          s += primeX2;
          // fast while loop (less operations)
          while (s + primeX6 * 5 + 8 < sieveEnd) {
            *s &= BIT5;
            s += primeX6;
            s[2] &= BIT1;
            s[primeX4 + 3] &= BIT2;
            s += primeX6;
            s[3] &= BIT6;
            s[primeX4 + 4] &= BIT7;
            s += primeX6;
            s[5] &= BIT3;
            s[primeX4 + 6] &= BIT4;
            s += primeX6 + 8;
            s[primeX4] &= BIT0;
            s += primeX6;
          }
          if (s >= sieveEnd)
            goto out0;
        }
        // for sieving primes of type n * 30 + 29
        for (;;) {
          case 49:
          *s &= BIT6;
          s += primeX6 + 3;
          if (s >= sieveEnd)
            goto out1;
          case 50:
          *s &= BIT5;
          s += primeX4 + 2;
          if (s >= sieveEnd)
            goto out2;
          case 51:
          *s &= BIT4;
          s += primeX2 + 1;
          if (s >= sieveEnd)
            goto out3;
          case 52:
          *s &= BIT3;
          s += primeX4 + 2;
          if (s >= sieveEnd)
            goto out4;
          case 53:
          *s &= BIT2;
          s += primeX2 + 1;
          if (s >= sieveEnd)
            goto out5;
          case 54:
          *s &= BIT1;
          s += primeX4 + 2;
          if (s >= sieveEnd)
            goto out6;
          case 55:
          *s &= BIT0;
          s += primeX6 + 2;
          if (s >= sieveEnd)
            goto out7;
          case 48:
          *s &= BIT7;
          s += primeX2 + 1;
          // fast while loop (less operations)
          while (s + primeX6 * 5 + 14 < sieveEnd) {
            *s &= BIT6;
            s += primeX6;
            s[3] &= BIT5;
            s[primeX4 + 5] &= BIT4;
            s += primeX6;
            s[6] &= BIT3;
            s[primeX4 + 8] &= BIT2;
            s += primeX6;
            s[9] &= BIT1;
            s[primeX4 + 11] &= BIT0;
            s += primeX6;
            s[primeX4 + 13] &= BIT7;
            s += primeX6 + 14;
          }
          if (s >= sieveEnd)
            goto out0;
        }
        // for sieving primes of type n * 30 + 31
        for (;;) {
          case 57:
          *s &= BIT7;
          s += primeX6 + 1;
          if (s >= sieveEnd)
            goto out1;
          case 58:
          *s &= BIT0;
          s += primeX4;
          if (s >= sieveEnd)
            goto out2;
          case 59:
          *s &= BIT1;
          s += primeX2;
          if (s >= sieveEnd)
            goto out3;
          case 60:
          *s &= BIT2;
          s += primeX4;
          if (s >= sieveEnd)
            goto out4;
          case 61:
          *s &= BIT3;
          s += primeX2;
          if (s >= sieveEnd)
            goto out5;
          case 62:
          *s &= BIT4;
          s += primeX4;
          if (s >= sieveEnd)
            goto out6;
          case 63:
          *s &= BIT5;
          s += primeX6;
          if (s >= sieveEnd)
            goto out7;
          case 56:
          *s &= BIT6;
          s += primeX2;
          // fast while loop (less operations)
          while (s + primeX6 * 5 + 1 < sieveEnd) {
            *s &= BIT7;
            s += primeX6 + 1;
            *s &= BIT0;
            s[primeX4] &= BIT1;
            s += primeX6;
            *s &= BIT2;
            s[primeX4] &= BIT3;
            s += primeX6;
            *s &= BIT4;
            s[primeX4] &= BIT5;
            s += primeX6;
            s[primeX4] &= BIT6;
            s += primeX6;
          }
          if (s >= sieveEnd)
            goto out0;
        }
        #define SIX_MOST_SIGNIFICANT_BITS 4227858432u
        /**
         * @def SET_WHEELINDEX(next_out)
         * Sets wheelIndex for the next sieve round.
         * i.e. if "goto out6;" happens at "case 46:" then wheelIndex is 
         * set to 47.
         */
        #define SET_WHEELINDEX(next_out) wPrime->index_ = \
          (wPrime->index_ & SIX_MOST_SIGNIFICANT_BITS) | (next_out << 23);
        
        out0: ; SET_WHEELINDEX(1); break;
        out1: ; SET_WHEELINDEX(2); break;
        out2: ; SET_WHEELINDEX(3); break;
        out3: ; SET_WHEELINDEX(4); break;
        out4: ; SET_WHEELINDEX(5); break;
        out5: ; SET_WHEELINDEX(6); break;
        out6: ; SET_WHEELINDEX(7); break;
        out7: ; wPrime->index_ &= SIX_MOST_SIGNIFICANT_BITS;
      }
      // set the sieveIndex for the next sieve round
      wPrime->setSieveIndex(static_cast<uint32_t> (s - sieveEnd));
    }
  }
}
