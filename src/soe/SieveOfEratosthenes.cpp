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

#include "SieveOfEratosthenes.h"
#include "PreSieve.h"
#include "EratSmall.h"
#include "EratMedium.h"
#include "EratBig.h"
#include "imath.h"

#include <stdint.h>
#include <stdexcept>
#include <cstdlib>
#include <algorithm>

namespace soe {

const uint_t SieveOfEratosthenes::bitValues_[8] = { 7, 11, 13, 17, 19, 23, 29, 31 };

/// De Bruijn sequence for first set bitValues_
const uint_t SieveOfEratosthenes::bruijnBitValues_[32] =
{
    7,  11, 109,  13, 113,  59,  97,  17,
  119,  89,  79,  61, 101,  71,  19,  37,
  121, 107,  53,  91,  83,  77,  67,  31,
  103,  49,  73,  29,  47,  23,  43,  41
};

/// @param start          Sieve the primes within the interval [start, stop].
/// @param stop           Sieve the primes within the interval [start, stop].
/// @param preSieveLimit  Multiples of small primes <= preSieveLimit are
///                       pre-sieved to speed up the sieve of Eratosthenes,
///                       preSieveLimit >= 13 && <= 23.
/// @param sieveSize      A sieve size in kilobytes, sieveSize >= 1 && <= 4096.
///
SieveOfEratosthenes::SieveOfEratosthenes(uint64_t start,
                                         uint64_t stop,
                                         uint_t preSieveLimit,
                                         uint_t sieveSize) :
  start_(start),
  stop_(stop),
  sqrtStop_(static_cast<uint_t>(isqrt(stop))),
  preSieve_(preSieveLimit),
  sieveSize_(std::max(1u, sieveSize) * 1024), // convert to bytes
  eratSmall_(NULL),
  eratMedium_(NULL),
  eratBig_(NULL)
{
  if (start_ < 7)
    throw std::invalid_argument("SieveOfEratosthenes: start must be >= 7.");
  if (start_ > stop_)
    throw std::invalid_argument("SieveOfEratosthenes: start must be <= stop.");
  segmentLow_ = start_ - getByteRemainder(start_);
  segmentHigh_ = segmentLow_ + sieveSize_ * NUMBERS_PER_BYTE + 1;
  initEratAlgorithms();
  // allocate the sieve of Eratosthenes array
  sieve_ = new uint8_t[sieveSize_ + 8];
}

SieveOfEratosthenes::~SieveOfEratosthenes() {
  delete eratSmall_;
  delete eratMedium_;
  delete eratBig_;
  delete[] sieve_;
}

void SieveOfEratosthenes::initEratAlgorithms() {
  try {
    if (sqrtStop_ > preSieve_.getLimit()) {
      eratSmall_ = new EratSmall(*this);
      if (sqrtStop_ > eratSmall_->getLimit()) {
        eratMedium_ = new EratMedium(*this);
        if (sqrtStop_ > eratMedium_->getLimit())
          eratBig_ = new EratBig(*this);
      }
    }
  } catch (...) {
    delete eratSmall_;
    delete eratMedium_;
    delete eratBig_;
    throw;
  }
}

uint64_t SieveOfEratosthenes::getByteRemainder(uint64_t n) {
  uint64_t remainder = n % NUMBERS_PER_BYTE;
  if (remainder <= 1)
    remainder += NUMBERS_PER_BYTE;
  return remainder;
}

/// Pre-sieve multiples of small primes <= preSieve_.getLimit()
/// to speed up the sieve of Eratosthenes.
/// @see PreSieve.cpp
///
void SieveOfEratosthenes::preSieve() {
  preSieve_.doIt(sieve_, sieveSize_, segmentLow_);
  // unset bits (numbers) < start_
  if (segmentLow_ <= start_) {
    if (start_ <= preSieve_.getLimit())
      sieve_[0] = 0xff;
    uint64_t remainder = getByteRemainder(start_);
    for (int i = 0; i < 8; i++)
      if (bitValues_[i] < remainder)
        sieve_[0] &= ~(1 << i);
  }
}

void SieveOfEratosthenes::crossOffMultiples() {
  if (eratSmall_ != NULL) {
    // process the sieving primes with many multiples per segment
    eratSmall_->crossOff(sieve_, &sieve_[sieveSize_]);
    if (eratMedium_ != NULL) {
      // process the sieving primes with a few multiples per segment
      eratMedium_->crossOff(sieve_, sieveSize_);
      if (eratBig_ != NULL)
        // process the sieving primes with very few ...
        eratBig_->crossOff(sieve_);
    }
  }
}

/// Sieve the primes within the current segment i.e.
/// [segmentLow_, segmentHigh_].
///
void SieveOfEratosthenes::sieveSegment() {
  preSieve();
  crossOffMultiples();
  segmentProcessed(sieve_, sieveSize_);
}

/// Sieve the last segments remaining after that sieve(prime) has
/// been called for all primes up to sqrt(stop_).
///
void SieveOfEratosthenes::finish() {
  // sieve all segments left except the last one
  while (segmentHigh_ < stop_) {
    sieveSegment();
    segmentLow_ += sieveSize_ * NUMBERS_PER_BYTE;
    segmentHigh_ += sieveSize_ * NUMBERS_PER_BYTE;
  }
  // sieve the last segment
  uint64_t remainder = getByteRemainder(stop_);
  sieveSize_ = static_cast<uint_t>((stop_ - remainder) - segmentLow_) / NUMBERS_PER_BYTE + 1;
  segmentHigh_ = segmentLow_ + sieveSize_ * NUMBERS_PER_BYTE + 1;
  preSieve();
  crossOffMultiples();
  // unset bits and bytes (numbers) > stop_
  for (int i = 0; i < 8; i++) {
    if (bitValues_[i] > remainder)
      sieve_[sieveSize_ - 1] &= ~(1 << i);
    sieve_[sieveSize_ + i] = 0;
  }
  segmentProcessed(sieve_, sieveSize_);
}

} // namespace soe
