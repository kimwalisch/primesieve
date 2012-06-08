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

#ifndef SIEVEOFERATOSTHENES_INLINE_H
#define SIEVEOFERATOSTHENES_INLINE_H

#include "config.h"
#include "SieveOfEratosthenes.h"
#include "EratSmall.h"
#include "EratMedium.h"
#include "EratBig.h"
#include "imath.h"

#include <stdint.h>

namespace soe {

inline uint64_t SieveOfEratosthenes::getStart() const     { return start_; }
inline uint64_t SieveOfEratosthenes::getStop() const      { return stop_; }
inline uint_t   SieveOfEratosthenes::getSqrtStop() const  { return sqrtStop_; }
inline uint_t   SieveOfEratosthenes::getPreSieve() const  { return preSieve_.getLimit(); }
inline uint_t   SieveOfEratosthenes::getSieveSize() const { return sieveSize_; }

/// Segmented sieve of Eratosthenes implementation.
/// sieve( prime ) must be called consecutively for all primes up to
/// sqrt(stop_) in order to sieve the primes within [start_, stop_].
///
inline void SieveOfEratosthenes::sieve(uint_t prime) {
  uint64_t square = isquare<uint64_t>(prime);
  // This loop segments the sieve of Eratosthenes, it is executed when
  // all primes <= sqrt(segmentHigh_) required to sieve the next
  // segment have been stored in the erat* objects below.
  // @see sieveSegment() in SieveOfEratosthenes.cpp.
  while (segmentHigh_ < square)
    sieveSegment();
  // add prime to eratSmall_ if it has many multiples per segment,
  // to eratMedium_ if it has a few multiples per segment or
  // to eratBig_ if it has very few multiples per segment.
  // @see addSievingPrime() in WheelFactorization.h
  if (prime > eratSmall_->getLimit()) 
    if (prime > eratMedium_->getLimit())
            eratBig_->addSievingPrime(prime, segmentLow_);
    else eratMedium_->addSievingPrime(prime, segmentLow_);
  else    eratSmall_->addSievingPrime(prime, segmentLow_);
}

/// Reconstruct the prime number corresponding to the first set
/// bit of the dword parameter (and unset bit).
/// @param index  The current sieve index.
/// @param dword  The next 4 bytes of the sieve array.
///
template <typename T>
inline T SieveOfEratosthenes::getNextPrime(uint_t index, uint_t* dword) const {
  // calculate bitValues_[ bitScanForward(dword) ] using De Bruijn bitscan
  uint_t firstBit = *dword & -static_cast<int>(*dword);
  uint_t bitValue = bruijnBitValues_[(firstBit * 0x077CB531) >> 27];
  T prime = static_cast<T>(segmentLow_ + index * NUMBERS_PER_BYTE + bitValue);
  *dword ^= firstBit;
  return prime;
}

} // namespace soe

#endif
