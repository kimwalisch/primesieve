//
// Copyright (c) 2011 Kim Walisch, <kim.walisch@gmail.com>.
// All rights reserved.
//
// This file is part of primesieve.
// Visit: http://primesieve.googlecode.com
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

#include "EratMedium.h"
#include "SieveOfEratosthenes.h"
#include "EratBase.h"
#include "WheelFactorization.h"
#include "defs.h"

#include <cstdlib>

EratMedium::EratMedium(uint32_t limit, const SieveOfEratosthenes& soe) :
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
    uint32_t    count       = bucket->getCount();
    WheelPrime* wheelPrimes = bucket->getWheelPrimes();

    for (uint32_t i = 0; i < count; i++) {
      uint32_t sieveIndex = wheelPrimes[i].getSieveIndex();
      if (sieveIndex >= sieveSize) {
        // nothing else to do for sieving primes that do not have a
        // multiple occurrence in the current segment
        wheelPrimes[i].indexes_ -= sieveSize;
        continue;
      }
      uint32_t sievingPrime = wheelPrimes[i].getSievingPrime();
      uint32_t wheelIndex   = wheelPrimes[i].getWheelIndex();
      // remove the multiples of the current sievingPrime from the
      // sieve array (i.e. the current segment)
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
      wheelPrimes[i].setIndexes(sieveIndex - sieveSize, wheelIndex);
    }
  }
}
