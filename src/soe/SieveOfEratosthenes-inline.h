///
/// @file   SieveOfEratosthenes-inline.h
/// @brief  Inline methods of the SieveOfEratosthenes class.
///
/// Copyright (C) 2012 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is licensed under the New BSD License. See the LICENSE
/// file in the top-level directory.
///

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
  // added to the erat* objects below.
  while (segmentHigh_ < square) {
    sieveSegment();
    segmentLow_  += sieveSize_ * NUMBERS_PER_BYTE;
    segmentHigh_ += sieveSize_ * NUMBERS_PER_BYTE;
  }
  if (prime > maxSmallPrime())
    if (prime > maxMediumPrime())
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
  uint64_t debruijn64 = UINT64_C(0x3F08A4C6ACB9DBD);
  uint64_t mask = *bits - 1;
  uint64_t bitValue = bruijnBitValues_[((*bits ^ mask) * debruijn64) >> 58];
  uint64_t byteValue = index * NUMBERS_PER_BYTE;
  uint64_t prime = segmentLow_ + byteValue + bitValue;
  *bits &= mask;
  return prime;
}

inline uint64_t    SieveOfEratosthenes::getStart()       const { return start_; }
inline uint64_t    SieveOfEratosthenes::getStop()        const { return stop_; }
inline uint_t      SieveOfEratosthenes::getSqrtStop()    const { return sqrtStop_; }
inline uint_t      SieveOfEratosthenes::getPreSieve()    const { return preSieve_.getLimit(); }
inline uint_t      SieveOfEratosthenes::getSieveSize()   const { return sieveSize_; }
inline uint_t      SieveOfEratosthenes::maxSmallPrime()  const { return eratSmall_->getLimit(); }
inline uint_t      SieveOfEratosthenes::maxMediumPrime() const { return eratMedium_->getLimit(); }
inline std::string SieveOfEratosthenes::getMaxStopString()     { return EratBig::getMaxStopString(); }
inline uint64_t    SieveOfEratosthenes::getMaxStop()           { return EratBig::getMaxStop(); }

} // namespace soe

#endif
