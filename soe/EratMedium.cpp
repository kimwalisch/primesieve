/*
 * EratMedium.cpp -- This file is part of primesieve
 *
 * Copyright (C) 2011 Kim Walisch, <kim.walisch@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include "EratMedium.h"
#include "SieveOfEratosthenes.h"
#include "EratBase.h"
#include "WheelFactorization.h"
#include "defs.h"

#include <cstdlib>

EratMedium::EratMedium(uint32_t limit, const SieveOfEratosthenes* soe) :
  EratBase<Modulo210Wheel> (limit, soe) {
}

/**
 * Implementation of the segmented sieve of Eratosthenes with wheel
 * factorization (modulo 210 wheel) and 30 numbers per byte.
 * This algorithm is optimized for sieving primes with few multiple
 * occurrences per segment.
 *
 * Removes the multiples (of sieving primes within EratMedium) from the
 * current segment.
 */
void EratMedium::sieve(uint8_t* sieve, uint32_t sieveSize) {
  // iterate over the sieving primes within EratMedium
  for (Bucket_t* bucket = bucketList_; bucket != NULL; bucket = bucket->next) {
    WheelPrime* wPrime = bucket->wheelPrimeBegin();
    WheelPrime* end = bucket->wheelPrimeEnd();
    for (; wPrime != end; wPrime++) {
      uint32_t sieveIndex = wPrime->getSieveIndex();
      if (sieveIndex >= sieveSize) {
        // nothing else to do for sieving primes that do not have a
        // multiple occurrence in the current segment
        wPrime->index_ -= sieveSize;
      } else {
        uint32_t sievingPrime = wPrime->getSievingPrime();
        uint32_t wheelIndex = wPrime->getWheelIndex();
        // remove the multiples of the current sievingPrime from the
        // sieve (i.e. the current segment)
        do {
          uint8_t bit = wheel_[wheelIndex].unsetBit;
          uint8_t nmf = wheel_[wheelIndex].nextMultipleFactor;
          uint8_t cor = wheel_[wheelIndex].correct;
           int8_t nxt = wheel_[wheelIndex].next;
          sieve[sieveIndex] &= bit;
          wheelIndex += nxt;
          sieveIndex += sievingPrime * nmf + cor;
        } while (sieveIndex < sieveSize);
        // set the sieveIndex and wheelIndex for the next segment
        sieveIndex -= sieveSize;
        wPrime->setWheelIndex(wheelIndex);
        wPrime->setSieveIndex(sieveIndex);
      }
    }
  }
}
