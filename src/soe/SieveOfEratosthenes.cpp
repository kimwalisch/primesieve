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
#include "config.h"
#include "imath.h"

#include <stdint.h>
#include <stdexcept>
#include <cstdlib>

namespace soe {

const uint32_t SieveOfEratosthenes::bitValues_[8] = { 7, 11, 13, 17, 19, 23, 29, 31 };

/** De Bruijn sequence for first set bitValues_ */
const uint32_t SieveOfEratosthenes::deBruijnFsbValues_[32] =
{
    7,  11, 109, 13, 113, 59, 97, 17,
  119,  89,  79, 61, 101, 71, 19, 37,
  121, 107,  53, 91,  83, 77, 67, 31,
  103,  49,  73, 29,  47, 23, 43, 41
};

/**
 * @param start          Sieve the primes within the interval [start, stop].
 * @param stop           Sieve the primes within the interval [start, stop].
 * @param preSieveLimit  Multiples of small primes <= preSieveLimit are
 *                       pre-sieved to speed up the sieve of Eratosthenes,
 *                       preSieveLimit >= 13 && <= 23.
 * @param sieveSize      A sieve size in kilobytes, sieveSize >= 1 && <= 4096.
 */
SieveOfEratosthenes::SieveOfEratosthenes(uint64_t start,
                                         uint64_t stop,
                                         uint32_t preSieveLimit,
                                         uint32_t sieveSize) :
  start_(start),
  stop_(stop),
  sqrtStop_(static_cast<uint32_t>(isqrt(stop))),
  preSieve_(preSieveLimit),
  isFirstSegment_(true),
  sieve_(NULL),
  sieveSize_(sieveSize * 1024),
  eratSmall_(NULL),
  eratMedium_(NULL),
  eratBig_(NULL)
{
  if (start_ < 7 || start_ > stop_)
    throw std::logic_error("SieveOfEratosthenes: start must be >= 7 && <= stop.");
  // it makes no sense to use very small sieve sizes
  if (sieveSize_ < 1024)
    throw std::invalid_argument("SieveOfEratosthenes: sieveSize must be >= 1 kilobyte.");
  segmentLow_ = start_ - getByteRemainder(start_);
  // '+ 1' is a correction for primes of type i*30 + 31
  segmentHigh_ = segmentLow_ + sieveSize_ * NUMBERS_PER_BYTE + 1;
  initEratAlgorithms();
  // allocate the sieve of Eratosthenes array
  sieve_ = new uint8_t[sieveSize_];
}

SieveOfEratosthenes::~SieveOfEratosthenes() {
  delete eratSmall_;
  delete eratMedium_;
  delete eratBig_;
  delete[] sieve_;
}

/** returns bitValues_[ bitScanForward(v) ] */
uint32_t SieveOfEratosthenes::getFirstSetBitValue(uint32_t v) {
  return deBruijnFsbValues_[((v & -static_cast<int32_t>(v)) * 0x077CB531U) >> 27];
}

uint32_t SieveOfEratosthenes::getPreSieveLimit() const {
  return preSieve_.getLimit();
}

uint32_t SieveOfEratosthenes::getByteRemainder(uint64_t n) const {
  uint32_t remainder = static_cast<uint32_t>(n % NUMBERS_PER_BYTE);
  // correction for primes of type i*30 + 31
  if (remainder <= 1)
    remainder += NUMBERS_PER_BYTE;
  return remainder;
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

/**
 * Pre-sieve multiples of small primes <= preSieve_.getLimit()
 * to speed up the sieve of Eratosthenes.
 */
void SieveOfEratosthenes::preSieve() {
  preSieve_.doIt(sieve_, sieveSize_, segmentLow_);
  if (isFirstSegment_) {
    isFirstSegment_ = false;
    // correct preSieve_.doIt() for numbers <= 23
    if (start_ <= preSieve_.getLimit())
      sieve_[0] = 0xff;
    uint32_t startRemainder = getByteRemainder(start_);
    // unset bits corresponding to numbers < start_
    for (int i = 0; i < 8; i++) {
      if (bitValues_[i] < startRemainder)
        sieve_[0] &= ~(1 << i);
    }
  }
}

/**
 * Cross-off the multiples within the current segment i.e.
 * [segmentLow_+7, segmentHigh_].
 */
void SieveOfEratosthenes::crossOffMultiples() {
  if (eratSmall_ != NULL) {
    // process the sieving primes with many multiples per segment
    eratSmall_->sieve(sieve_, sieveSize_);
    if (eratMedium_ != NULL) {
      // process the sieving primes with a few multiples per segment
      eratMedium_->sieve(sieve_, sieveSize_);
      if (eratBig_ != NULL)
        // process the sieving primes with very few multiples per segment
        eratBig_->sieve(sieve_);
    }
  }
}

/**
 * Implementation of the segmented sieve of Eratosthenes.
 * sieve(uint32_t) must be called consecutively for all primes up to
 * sqrt(stop) in order to sieve the primes within the interval
 * [start_, stop_].
 */
void SieveOfEratosthenes::sieve(uint32_t prime) {
  const uint64_t primeSquared = isquare<uint64_t>(prime);

  // The following while loop segments the sieve of Eratosthenes, it
  // is executed when all sieving primes <= sqrt(segmentHigh_)
  // required to sieve the next segment have been stored in the erat*
  // objects (see below). Each loop iteration sieves the primes within
  // the interval [segmentLow_+7, segmentHigh_].
  while (segmentHigh_ < primeSquared) {
    preSieve();
    crossOffMultiples();
    analyseSieve(sieve_, sieveSize_);
    segmentLow_ += sieveSize_ * NUMBERS_PER_BYTE;
    segmentHigh_ += sieveSize_ * NUMBERS_PER_BYTE;
  }

  // prime is added to eratSmall_ if it has many multiples per
  // segment, to eratMedium_ if it has a few multiples per segment
  // and to eratBig_ if it has very few multiples per segment.
  if (prime > eratSmall_->getLimit())
    if (prime > eratMedium_->getLimit())
            eratBig_->addSievingPrime(prime);
    else eratMedium_->addSievingPrime(prime);
  else    eratSmall_->addSievingPrime(prime);
}

/**
 * Sieve the last segments remaining after that sieve(uint32_t)
 * has been called for all primes up to sqrt(stop_).
 */
void SieveOfEratosthenes::finish() {
  // sieve all segments left except the last one
  while (segmentHigh_ < stop_) {
    preSieve();
    crossOffMultiples();
    analyseSieve(sieve_, sieveSize_);
    segmentLow_ += sieveSize_ * NUMBERS_PER_BYTE;
    segmentHigh_ += sieveSize_ * NUMBERS_PER_BYTE;
  }
  uint32_t stopRemainder = getByteRemainder(stop_);
  // calculate the sieve size of the last segment
  sieveSize_ = static_cast<uint32_t>((stop_ - stopRemainder) - segmentLow_) / NUMBERS_PER_BYTE + 1;
  // sieve the last segment
  preSieve();
  crossOffMultiples();
  // unset bits corresponding to numbers > stop_
  for (int i = 0; i < 8; i++) {
    if (bitValues_[i] > stopRemainder)
      sieve_[sieveSize_ - 1] &= ~(1 << i);
  }
  analyseSieve(sieve_, sieveSize_);
}

} // namespace soe
