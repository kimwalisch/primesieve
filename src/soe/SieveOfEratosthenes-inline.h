///
/// @file   SieveOfEratosthenes-inline.h
/// @brief  Inline methods of the SieveOfEratosthenes class.
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

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

inline uint64_t SieveOfEratosthenes::getStart()     const { return start_; }
inline uint64_t SieveOfEratosthenes::getStop()      const { return stop_; }
inline uint_t   SieveOfEratosthenes::getSieveSize() const { return sieveSize_; }

/// Reconstruct the prime number corresponding to the first set
/// bit of the `bits' parameter and unset that bit.
/// @see SieveOfEratosthenes-GENERATE.h
///
inline uint64_t SieveOfEratosthenes::getNextPrime(uint64_t* bits, uint_t index) const
{
  // calculate bitValues_[ bitScanForward(*bits) ]
  // using a custom De Bruijn bitscan
  uint64_t debruijn64 = UINT64_C(0x3F08A4C6ACB9DBD);
  uint64_t mask = *bits - 1;
  uint64_t bitValue = bruijnBitValues_[((*bits ^ mask) * debruijn64) >> 58];
  uint64_t byteValue = index * NUMBERS_PER_BYTE;
  uint64_t prime = segmentLow_ + byteValue + bitValue;
  *bits &= mask;
  return prime;
}

/// This method must be called consecutively for all primes up to
/// sqrt(stop) in order to sieve the primes within the
/// interval [start, stop].
///
inline void SieveOfEratosthenes::addSievingPrime(uint_t prime)
{
  uint64_t square = isquare<uint64_t>(prime);
  // This loop is executed once all primes <= sqrt(segmentHigh_)
  // required to sieve the next segment have been
  // added to the erat* objects further down.
  while (segmentHigh_ < square) {
    sieveSegment();
    segmentLow_  += sieveSize_ * NUMBERS_PER_BYTE;
    segmentHigh_ += sieveSize_ * NUMBERS_PER_BYTE;
  }
       if (prime > limitEratMedium_)   eratBig_->addSievingPrime(prime, segmentLow_);
  else if (prime > limitEratSmall_) eratMedium_->addSievingPrime(prime, segmentLow_);
  else /* (prime > limitPreSieve) */ eratSmall_->addSievingPrime(prime, segmentLow_);
}

} // namespace soe

#endif
