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
#include "PreSieve.h"
#include "EratSmall.h"
#include "EratMedium.h"
#include "EratBig.h"
#include "imath.h"

#include <stdint.h>
#include <string>
#include <cassert>

namespace soe {

inline uint64_t    SieveOfEratosthenes::getMaxStop()         { return EratBig::getMaxStop(); }
inline std::string SieveOfEratosthenes::getMaxStopString()   { return EratBig::getMaxStopString(); }
inline uint64_t    SieveOfEratosthenes::getStart() const     { return start_; }
inline uint64_t    SieveOfEratosthenes::getStop() const      { return stop_; }
inline uint_t      SieveOfEratosthenes::getSqrtStop() const  { return sqrtStop_; }
inline uint_t      SieveOfEratosthenes::getPreSieve() const  { return preSieve_.getLimit(); }
inline uint_t      SieveOfEratosthenes::getSieveSize() const { return sieveSize_; }

/// Sieve primes using the segmented sieve of Eratosthenes.
/// sieve(uint_t prime) must be called consecutively for all primes
/// up to sqrt(stop) in order to sieve the primes within the
/// interval [start, stop].
///
inline void SieveOfEratosthenes::sieve(uint_t prime)
{
  assert(prime <= sqrtStop_);
  uint64_t square = isquare<uint64_t>(prime);
  // This loop is executed once all primes <= sqrt(segmentHigh_)
  // required to sieve the next segment have been
  // stored in the erat* objects below.
  while (segmentHigh_ < square) {
    sieveSegment();
    segmentLow_ += sieveSize_ * NUMBERS_PER_BYTE;
    segmentHigh_ += sieveSize_ * NUMBERS_PER_BYTE;
  }
  if (prime > eratSmall_->getLimit())
    if (prime > eratMedium_->getLimit())
            eratBig_->add(prime, segmentLow_);
    else eratMedium_->add(prime, segmentLow_);
  else    eratSmall_->add(prime, segmentLow_);
}

/// Reconstruct the prime number corresponding to the first set
/// bit of the `bits' parameter and unset that bit.
/// @see GENERATE.h.
///
inline uint64_t SieveOfEratosthenes::getNextPrime(uint64_t* bits, uint_t index) const
{
  // calculate bitValues_[ bitScanForward(*bits) ]
  // using a custom De Bruijn bitscan
  uint64_t mask = *bits - 1;
  uint64_t bitValue = bruijnBitValues_[((*bits ^ mask) * UINT64_C(0x3F08A4C6ACB9DBD)) >> 58];
  uint64_t byteValue = index * NUMBERS_PER_BYTE;
  uint64_t prime = segmentLow_ + byteValue + bitValue;
  *bits &= mask;
  return prime;
}

} // namespace soe

#endif
