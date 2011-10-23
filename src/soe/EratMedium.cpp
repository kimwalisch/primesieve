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

#include "EratMedium.h"
#include "SieveOfEratosthenes.h"
#include "EratBase.h"
#include "WheelFactorization.h"
#include "defs.h"

#include <algorithm>
#include <list>

EratMedium::EratMedium(const SieveOfEratosthenes& soe) :
  EratBase<Modulo210Wheel, WheelPrime_2> (soe)
{
  static_assert(defs::ERATMEDIUM_FACTOR <= 15, "defs::ERATMEDIUM_FACTOR <= 15");
  uint32_t sqrtStop = soe.getSquareRoot();
  uint32_t max      = static_cast<uint32_t> (soe.getSieveSize() * defs::ERATMEDIUM_FACTOR);
  uint32_t limit    = std::min<uint32_t>(sqrtStop, max);
  this->setLimit(limit);
}

/**
 * Implementation of the segmented sieve of Eratosthenes with wheel
 * factorization optimized for medium sieving primes with a few
 * multiples per segment.
 * This implementation uses a sieve array with 30 numbers per byte and
 * a modulo 210 wheel that skips multiples of 2, 3, 5 and 7.
 * @see SieveOfEratosthenes::crossOffMultiples()
 */
void EratMedium::sieve(uint8_t* sieve, uint32_t sieveSize)
{
  std::list<Bucket_t>::iterator bucket     = buckets_.begin();
  std::list<Bucket_t>::iterator bucketsEnd = buckets_.end();
  for (; bucket != bucketsEnd; ++bucket)
  {
    WheelPrime_t* wPrime = bucket->begin();
    WheelPrime_t* end    = bucket->end();
    // remove the multiples of sieving primes within the
    // current bucket from the sieve array
    for (; wPrime != end; wPrime++) {
      uint32_t sievingPrime = wPrime->getSievingPrime();
      uint32_t wheelIndex   = wPrime->getWheelIndex();
      uint32_t sieveIndex   = wPrime->getSieveIndex(); 
      // cross-off the multiples (unset corresponding bits) of the
      // current sieving prime within the sieve array
      while (sieveIndex < sieveSize) {
        uint32_t unsetBit = wheel_[wheelIndex].unsetBit;
        uint32_t factor   = wheel_[wheelIndex].nextMultipleFactor;
        uint32_t correct  = wheel_[wheelIndex].correct;
         int32_t next     = wheel_[wheelIndex].next;
        sieve[sieveIndex] &= unsetBit;
        wheelIndex += next;
        sieveIndex += sievingPrime * factor + correct;
      }
      sieveIndex -= sieveSize;
      // set sieveIndex and wheelIndex for the next segment
      wPrime->set(sievingPrime, sieveIndex, wheelIndex);
    }
  }
}
