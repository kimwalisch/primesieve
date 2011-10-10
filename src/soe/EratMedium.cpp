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
#include "imath.h"
#include "defs.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>

EratMedium::EratMedium(const SieveOfEratosthenes& soe) :
  EratBase<Modulo210Wheel, WheelPrime_2> (soe)
{
  uint32_t sqrtStop = isqrt(soe.getStopNumber());
  uint32_t max      = soe.getSieveSize() * 15;
  uint32_t limit    = std::min<uint32_t>(sqrtStop, max);
  assert(limit <= (1U << 23) * 15);
  this->setLimit(limit);
}

/**
 * Implementation of the segmented sieve of Eratosthenes with wheel
 * factorization (modulo 210 wheel) and 30 numbers per byte.
 * This algorithm is optimized for sieving primes with few multiple
 * occurrences per segment.
 *
 * Removes the multiples of sieving primes within EratMedium from
 * the current segment.
 * @see SieveOfEratosthenes::crossOffMultiples()
 */
void EratMedium::sieve(uint8_t* sieve, uint32_t sieveSize) {
  // iterate over the sieving primes within EratMedium
  for (Bucket_t* bucket = bucketList_; bucket != NULL; bucket = bucket->next) {
    uint32_t      count       = bucket->getCount();
    WheelPrime_t* wheelPrimes = bucket->getWheelPrimes();

    for (uint32_t i = 0; i < count; i++) {
      if (wheelPrimes[i].sieveIndex_ >= sieveSize) {
        // the current sievingPrime does not have a multiple
        // occurrence in the current segment
        wheelPrimes[i].sieveIndex_ -= sieveSize;
        continue;
      }
      uint32_t sieveIndex   = wheelPrimes[i].getSieveIndex();
      uint32_t wheelIndex   = wheelPrimes[i].getWheelIndex();
      uint32_t sievingPrime = wheelPrimes[i].getSievingPrime();
      // cross off the multiples (unset corresponding bits) of the
      // current sievingPrime within the sieve array
      do {
        uint8_t unsetBit   = wheel_[wheelIndex].unsetBit;
        uint8_t nextFactor = wheel_[wheelIndex].nextMultipleFactor;
        uint8_t correct    = wheel_[wheelIndex].correct;
         int8_t next       = wheel_[wheelIndex].next;
        wheelIndex += next;
        sieve[sieveIndex] &= unsetBit;
        sieveIndex += sievingPrime * nextFactor + correct;
      } while (sieveIndex < sieveSize);
      sieveIndex -= sieveSize;
      // set sieveIndex and wheelIndex for the next segment
      wheelPrimes[i].set(sievingPrime, sieveIndex, wheelIndex);
    }
  }
}
