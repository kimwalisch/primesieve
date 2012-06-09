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
#include "SieveOfEratosthenes-inline.h"
#include "WheelFactorization.h"
#include "config.h"

#include <stdint.h>
#include <stdexcept>
#include <algorithm>
#include <list>

namespace soe {

EratMedium::EratMedium(const SieveOfEratosthenes& soe) :
  Modulo210Wheel_t(soe), buckets_(1, Bucket())
{
  // assert multipleIndex < 2^23 in crossOff()
  static_assert(config::FACTOR_ERATMEDIUM <= 6, "config::FACTOR_ERATMEDIUM must not be > 6");
  if (soe.getSieveSize() > (1u << 22))
    throw std::overflow_error("EratMedium: sieveSize must be <= 2^22, 4096 kilobytes.");
  uint_t sqrtStop = soe.getSqrtStop();
  uint_t max      = soe.getSieveSize() * config::FACTOR_ERATMEDIUM;
  limit_          = std::min(sqrtStop, max);
}

/// Add a new sieving prime
/// @see addSievingPrime() in WheelFactorization.h
///
void EratMedium::storeSievingPrime(uint_t prime, uint_t multipleIndex, uint_t wheelIndex)
{
  if (!buckets_.back().store(prime, multipleIndex, wheelIndex))
    buckets_.push_back(Bucket());
}

/// This is an implementation of the segmented sieve of Eratosthenes
/// with wheel factorization optimized for medium sieving primes that
/// have a few multiples per segment. EratMedium uses a modulo 210
/// wheel that skips multiples of 2, 3, 5 and 7.
/// @see crossOffMultiples() in SieveOfEratosthenes.cpp
///
void EratMedium::crossOff(uint8_t* sieve, uint_t sieveSize)
{
  for (BucketList_t::iterator bucket = buckets_.begin(); bucket != buckets_.end(); ++bucket) {
    WheelPrime* wPrime = bucket->begin();
    WheelPrime* end    = bucket->end();
    // 2 sieving primes are processed per loop iteration to break the
    // dependency chain and reduce pipeline stalls
    for (; wPrime + 2 <= end; wPrime += 2) {
      uint_t multipleIndex0 = wPrime[0].getMultipleIndex();
      uint_t wheelIndex0    = wPrime[0].getWheelIndex();
      uint_t sievingPrime0  = wPrime[0].getSievingPrime();
      uint_t multipleIndex1 = wPrime[1].getMultipleIndex();
      uint_t wheelIndex1    = wPrime[1].getWheelIndex();
      uint_t sievingPrime1  = wPrime[1].getSievingPrime();
      // cross-off the multiples (unset bits) of sievingPrime(0|1)
      // @see unsetBit() in WheelFactorization.h
      for (;;) {
         if (multipleIndex0 < sieveSize) unsetBit(sieve, sievingPrime0, &multipleIndex0, &wheelIndex0); else break;
         if (multipleIndex1 < sieveSize) unsetBit(sieve, sievingPrime1, &multipleIndex1, &wheelIndex1); else break;
      }
      while (multipleIndex0 < sieveSize) unsetBit(sieve, sievingPrime0, &multipleIndex0, &wheelIndex0);
      while (multipleIndex1 < sieveSize) unsetBit(sieve, sievingPrime1, &multipleIndex1, &wheelIndex1);
      multipleIndex0 -= sieveSize;
      multipleIndex1 -= sieveSize;
      wPrime[0].set(multipleIndex0, wheelIndex0);
      wPrime[1].set(multipleIndex1, wheelIndex1);
    }
    if (wPrime != end) {
      uint_t multipleIndex = wPrime->getMultipleIndex();
      uint_t wheelIndex    = wPrime->getWheelIndex();
      uint_t sievingPrime  = wPrime->getSievingPrime();
      while (multipleIndex < sieveSize)
        unsetBit(sieve, sievingPrime, &multipleIndex, &wheelIndex);
      multipleIndex -= sieveSize;
      wPrime->set(multipleIndex, wheelIndex);
    }
  }
}

} // namespace soe
