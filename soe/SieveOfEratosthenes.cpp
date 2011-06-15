/*
 * SieveOfEratosthenes.cpp -- This file is part of primesieve
 *
 * Copyright (C) 2011 Kim Walisch, <kim.walisch@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include "SieveOfEratosthenes.h"
#include "ResetSieve.h"
#include "EratSmall.h"
#include "EratMedium.h"
#include "EratBig.h"
#include "defs.h"
#include "pmath.h"

#include <stdexcept>
#include <cstdlib>
#include <algorithm>
#include <cassert>

const uint32_t SieveOfEratosthenes::bitValues_[8] = { 7, 11, 13, 17, 19, 23, 29, 31 };

/**
 * @param startNumber A start number for sieving.
 * @param stopNumber  A stop number for sieving.
 * @param sieveSize   A sieve size in bytes.
 * @param resetSieve  A ResetSieve object that is used to reset the
 *                    sieve_ array.
 */
SieveOfEratosthenes::SieveOfEratosthenes(uint64_t startNumber,
    uint64_t stopNumber, uint32_t sieveSize, ResetSieve* resetSieve) :
  startNumber_(startNumber), stopNumber_(stopNumber), sieve_(NULL),
      sieveSize_(sieveSize), resetSieve_(resetSieve), eratSmall_(NULL),
           eratMedium_(NULL), eratBig_(NULL) {
  if (startNumber_ < 7 || startNumber_ > stopNumber_)
    throw std::logic_error(
        "SieveOfEratosthenes: startNumber must be >= 7 && <= stop number.");
  // it makes no sense to use very small sieve sizes as a sieve size
  // of the CPU's L1 or L2 cache size performs best
  if (sieveSize_ < 1024)
    throw std::invalid_argument(
        "SieveOfEratosthenes: sieveSize must be >= 1024.");
  if (sieveSize_ > UINT32_MAX / NUMBERS_PER_BYTE)
    throw std::overflow_error(
        "SieveOfEratosthenes: sieveSize must be <= 2^32 / 30.");
  lowerBound_ = startNumber - this->getRemainder(startNumber);
  assert(lowerBound_ % NUMBERS_PER_BYTE == 0);
  resetIndex_ = resetSieve_->getResetIndex(lowerBound_);
  try {
    this->initEratAlgorithms();
    this->initSieve();
  } catch (...) {
    delete[] sieve_;
    delete eratSmall_;
    delete eratMedium_;
    delete eratBig_;
    throw;
  }
}

SieveOfEratosthenes::~SieveOfEratosthenes() {
  delete[] sieve_;
  delete eratSmall_;
  delete eratMedium_;
  delete eratBig_;
}

uint32_t SieveOfEratosthenes::getRemainder(uint64_t n) {
  uint32_t remainder = static_cast<uint32_t> (n % NUMBERS_PER_BYTE);
  return (remainder > 1)
      ? remainder 
      : remainder + NUMBERS_PER_BYTE;
}

/**
 * Allocate the sieve array and set its bits to 1.
 */
void SieveOfEratosthenes::initSieve() {
  sieve_ = new uint8_t[sieveSize_];

  uint64_t bytesToSieve = (stopNumber_ - lowerBound_) / NUMBERS_PER_BYTE + 1;
  uint32_t resetSize = (sieveSize_ < bytesToSieve) ? sieveSize_
      : static_cast<uint32_t> (bytesToSieve);
  resetSieve_->reset(sieve_, resetSize, &resetIndex_);
  // correct the first byte of the sieve_ for numbers <= 31
  if (startNumber_ <= resetSieve_->getLimit())
    sieve_[0] = 0xff;

  uint32_t startRemainder = this->getRemainder(startNumber_);
  // unset the bits of the first byte corresponding to
  // numbers < startNumber_
  uint32_t i = 0;
  while (i < 8 && bitValues_[i] < startRemainder)
    i++;
  sieve_[0] &= static_cast<uint8_t> (0xff << i);
}

/**
 * Initialize the 3 erat* algorithms if needed.
 */
void SieveOfEratosthenes::initEratAlgorithms() {
  assert(defs::FACTOR_ERATSMALL <= defs::FACTOR_ERATMEDIUM);
  uint32_t sq = isqrt(stopNumber_);
  uint32_t limit;
  if (resetSieve_->getLimit() < sq) {
    limit = static_cast<uint32_t> (sieveSize_* defs::FACTOR_ERATSMALL);
    eratSmall_ = new EratSmall(std::min<uint32_t>(limit, sq), this);
    if (eratSmall_->getLimit() < sq) {
      limit = static_cast<uint32_t> (sieveSize_* defs::FACTOR_ERATMEDIUM);
      eratMedium_ = new EratMedium(std::min<uint32_t>(limit, sq), this);
      if (eratMedium_->getLimit() < sq)
        eratBig_ = new EratBig(this);
    }
  }
}

/**
 * Use the erat* algorithms to cross-off the multiples within the
 * current segment.
 */
void SieveOfEratosthenes::crossOffMultiples() {
  if (eratSmall_ != NULL) {
    eratSmall_->sieve(sieve_, sieveSize_);
    if (eratMedium_ != NULL) {
      eratMedium_->sieve(sieve_, sieveSize_);
      if (eratBig_ != NULL)
        eratBig_->sieve(sieve_);
    }
  }
}

/**
 * Implementation of the segmented sieve of Eratosthenes.
 * sieve(uint32_t) must be called consecutively for all primes up to
 * stopNumber_^0.5 in order to sieve the primes in the interval
 * [startNumber_, stopNumber_].
 */
void SieveOfEratosthenes::sieve(uint32_t primeNumber) {
  assert(eratSmall_ != NULL && 
      primeNumber > resetSieve_->getLimit() &&
      isquare(primeNumber) <= stopNumber_);
  /// @remark '- 6' is a correction for primes of type n * 30 + 31
  const uint64_t primeSquared = isquare(primeNumber) - 6;

  // the following while loop is entered if all primes required to
  // sieve the next segment are present in the erat* objects
  while (lowerBound_ + sieveSize_ * NUMBERS_PER_BYTE < primeSquared) {
    this->crossOffMultiples();
    this->analyseSieve(sieve_, sieveSize_);
    resetSieve_->reset(sieve_, sieveSize_, &resetIndex_);
    lowerBound_ += sieveSize_ * NUMBERS_PER_BYTE;
  }
  // add primeNumber to the appropriate erat* object
  if (primeNumber > eratSmall_->getLimit())
    if (primeNumber > eratMedium_->getLimit())
            eratBig_->addPrimeNumber(primeNumber, lowerBound_);
    else eratMedium_->addPrimeNumber(primeNumber, lowerBound_);
  else    eratSmall_->addPrimeNumber(primeNumber, lowerBound_);
}

/**
 * Sieve the last segments remaining after that sieve(uint32_t) has
 * been called consecutively for all prime numbers up to
 * stopNumber_^0.5.
 */
void SieveOfEratosthenes::finish() {
  assert(lowerBound_ < stopNumber_);
  /// sieve all segments left except the last one
  /// @remark '+ 1' is a correction for primes of type n * 30 + 31
  while (lowerBound_ + sieveSize_ * NUMBERS_PER_BYTE + 1 < stopNumber_) {
    this->crossOffMultiples();
    this->analyseSieve(sieve_, sieveSize_);
    resetSieve_->reset(sieve_, sieveSize_, &resetIndex_);
    lowerBound_ += sieveSize_ * NUMBERS_PER_BYTE;
  }
  uint32_t stopRemainder = this->getRemainder(stopNumber_);
  // calculate the sieve size of the last segment
  sieveSize_ = static_cast<uint32_t> ((stopNumber_ - stopRemainder)
      - lowerBound_) / NUMBERS_PER_BYTE + 1;
  assert(lowerBound_ + (sieveSize_ - 1) * NUMBERS_PER_BYTE + stopRemainder
      == stopNumber_);
  // sieve the last segment
  this->crossOffMultiples();
  // unset the bits of the last byte corresponding to
  // numbers > stopNumber_
  for (uint32_t i = 0; i < 8; i++) {
    if (stopRemainder < bitValues_[i]) {
      sieve_[sieveSize_ - 1] &= 0xff >> (8 - i);
      break;
    }
  }
  this->analyseSieve(sieve_, sieveSize_);
}
