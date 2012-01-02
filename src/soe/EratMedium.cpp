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

#include "EratMedium.h"
#include "SieveOfEratosthenes.h"
#include "EratBase.h"
#include "WheelFactorization.h"
#include "defs.h"

#include <stdexcept>
#include <algorithm>
#include <list>

EratMedium::EratMedium(const SieveOfEratosthenes& soe) :
  EratBase<Modulo210Wheel_t> (soe)
{
  // conditions that assert multipleIndex < 2^23 in sieve()
  static_assert(defs::ERATMEDIUM_FACTOR <= 6, "defs::ERATMEDIUM_FACTOR <= 6");
  if (soe.getSieveSize() > (1U << 22))
    throw std::overflow_error(
        "EratMedium: sieveSize must be <= 2^22, 4096 kilobytes.");
  uint32_t sqrtStop = soe.getSquareRoot();
  uint32_t max      = soe.getSieveSize() * defs::ERATMEDIUM_FACTOR;
  uint32_t limit    = std::min(sqrtStop, max);
  this->setLimit(limit);
}

/**
 * Implementation of the segmented sieve of Eratosthenes with wheel
 * factorization optimized for medium sieving primes that have a few
 * multiples per segment.
 * This implementation uses a sieve array with 30 numbers per byte and
 * a modulo 210 wheel that skips multiples of 2, 3, 5 and 7.
 * @see SieveOfEratosthenes::crossOffMultiples()
 */
void EratMedium::sieve(uint8_t* sieve, uint32_t sieveSize)
{
  for (BucketList_t::iterator bucket = buckets_.begin(); bucket != buckets_.end(); ++bucket) {
    WheelPrime* wPrime    = bucket->begin();
    WheelPrime* const end = bucket->end();
    // For out-of-order CPUs this algorithm can be sped up by
    // processing 2 sieving primes per loop iteration, this breaks the
    // dependency chain and reduces pipeline stalls
    for (; wPrime + 2 <= end; wPrime += 2) {
      uint32_t multipleIndex0 = wPrime[0].getMultipleIndex();
      uint32_t wheelIndex0    = wPrime[0].getWheelIndex();
      uint32_t sievingPrime0  = wPrime[0].getSievingPrime();
      uint32_t multipleIndex1 = wPrime[1].getMultipleIndex();
      uint32_t wheelIndex1    = wPrime[1].getWheelIndex();
      uint32_t sievingPrime1  = wPrime[1].getSievingPrime();
      while (multipleIndex0 < sieveSize &&
             multipleIndex1 < sieveSize) {
        // cross-off the next multiple (unset corresponding bit) of
        // the current sieving primes within the sieve array
        sieve[multipleIndex0] &= wheel(wheelIndex0)->unsetBit;
        multipleIndex0        += wheel(wheelIndex0)->nextMultipleFactor * sievingPrime0;
        multipleIndex0        += wheel(wheelIndex0)->correct;
        wheelIndex0           += wheel(wheelIndex0)->next;
        sieve[multipleIndex1] &= wheel(wheelIndex1)->unsetBit;
        multipleIndex1        += wheel(wheelIndex1)->nextMultipleFactor * sievingPrime1;
        multipleIndex1        += wheel(wheelIndex1)->correct;
        wheelIndex1           += wheel(wheelIndex1)->next;
      }
      while (multipleIndex0 < sieveSize) {
        sieve[multipleIndex0] &= wheel(wheelIndex0)->unsetBit;
        multipleIndex0        += wheel(wheelIndex0)->nextMultipleFactor * sievingPrime0;
        multipleIndex0        += wheel(wheelIndex0)->correct;
        wheelIndex0           += wheel(wheelIndex0)->next;
      }
      while (multipleIndex1 < sieveSize) {
        sieve[multipleIndex1] &= wheel(wheelIndex1)->unsetBit;
        multipleIndex1        += wheel(wheelIndex1)->nextMultipleFactor * sievingPrime1;
        multipleIndex1        += wheel(wheelIndex1)->correct;
        wheelIndex1           += wheel(wheelIndex1)->next;
      }
      multipleIndex0 -= sieveSize;
      multipleIndex1 -= sieveSize;
      // set the multipleIndex and wheelIndex for the next segment
      wPrime[0].setIndexes(multipleIndex0, wheelIndex0);
      wPrime[1].setIndexes(multipleIndex1, wheelIndex1);
    }
    if (wPrime != end) {
      uint32_t multipleIndex = wPrime->getMultipleIndex();
      uint32_t wheelIndex    = wPrime->getWheelIndex();
      uint32_t sievingPrime  = wPrime->getSievingPrime();
      while (multipleIndex < sieveSize) {
        sieve[multipleIndex] &= wheel(wheelIndex)->unsetBit;
        multipleIndex        += wheel(wheelIndex)->nextMultipleFactor * sievingPrime;
        multipleIndex        += wheel(wheelIndex)->correct;
        wheelIndex           += wheel(wheelIndex)->next;
      }
      multipleIndex -= sieveSize;
      wPrime->setIndexes(multipleIndex, wheelIndex);
    }
  }
}
