/*
 * EratMedium.cpp -- This file is part of primesieve
 *
 * Copyright (C) 2010 Kim Walisch, <kim.walisch@gmail.com>
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

#include <stdint.h>
#include <cstdlib>

EratMedium::EratMedium(uint32_t limit, uint64_t stopNumber, uint32_t sieveSize) :
  EratBase<Modulo210Wheel> (limit, stopNumber, sieveSize) {
}

/**
 * Implementation of the segmented sieve of Eratosthenes with wheel
 * factorization (modulo 210 wheel). Is used to cross-off the
 * multiples of the current sieve round.
 */
void EratMedium::sieve(uint8_t* sieve, uint32_t sieveSize) {
  // iterate over all the buckets of the bucket list
  for (Bucket_t* bucket = bucketList_; bucket != NULL; bucket = bucket->next) {
    WheelPrime* wPrime = bucket->wheelPrimeBegin();
    WheelPrime* end    = bucket->wheelPrimeEnd();
    // iterate over the wheelPrimes of the current bucket
    for (; wPrime != end; wPrime++) {
      uint32_t sieveIndex = wPrime->getSieveIndex();
      // nothing to do for primes that do not have a multiple
      // occurence in the current sieve round
      if (sieveIndex >= sieveSize) {
        wPrime->index_ -= sieveSize;
      } else {
        uint32_t sievePrime = wPrime->getSievePrime();
        uint32_t wheelIndex = wPrime->getWheelIndex();
        // eliminate the multiples of the current wheelPrime (of the
        // current sieve round)
        do {
          uint8_t bit = wheel_[wheelIndex].unsetBit;
          uint8_t nmf = wheel_[wheelIndex].nextMultipleFactor;
          uint8_t cor = wheel_[wheelIndex].correct;
           int8_t nxt = wheel_[wheelIndex].next;
          sieve[sieveIndex] &= bit;
          wheelIndex += nxt;
          sieveIndex += sievePrime * nmf + cor;
        } while (sieveIndex < sieveSize);
        // sets the sieveIndex and wheelIndex for the next sieve round
        sieveIndex -= sieveSize;
        wPrime->setWheelIndex(wheelIndex);
        wPrime->setSieveIndex(sieveIndex);
      }
    }
  }
}
