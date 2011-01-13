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
#include "settings.h"
#include "pmath.h"

#include <stdint.h>
#include <stdexcept>
#include <cstdlib>
#include <algorithm>
#include <cassert>

const uint32_t SieveOfEratosthenes::bitValues_[8] = { 7, 11, 13, 17, 19, 23,
    29, 31 };

/**
 * @startNumber A start number for prime sieving.
 * @stopNumber  A stop number for prime sieving.
 * @sieveSize   A sieve size in bytes.
 * @resetSieve  A ResetSieve object used to reset the sieve_ array.
 */
SieveOfEratosthenes::SieveOfEratosthenes(uint64_t startNumber,
    uint64_t stopNumber, uint32_t sieveSize, ResetSieve* resetSieve) :
  startNumber_(startNumber), stopNumber_(stopNumber), sieve_(NULL),
      resetSieve_(resetSieve), eratSmall_(NULL), eratMedium_(NULL), eratBig_(
          NULL) {
  if (startNumber_ < 7)
    throw std::invalid_argument(
        "SieveOfEratosthenes: start number must be >= 7.");
  if (startNumber_ > stopNumber_)
    throw std::invalid_argument(
        "SieveOfEratosthenes: start number must be <= stop number.");
  this->setSieveSize(sieveSize);
  this->setLowerBound(startNumber);
  this->setResetIndex();
  try {
    this->initEratAlgorithms();
    this->initSieve();
  } catch (...) {
    this->free();
    throw;
  }
}

SieveOfEratosthenes::~SieveOfEratosthenes() {
  this->free();
}

void SieveOfEratosthenes::free() {
  if (sieve_ != NULL) {
    delete[] sieve_;
    sieve_ = NULL;
  }
  if (eratSmall_ != NULL) {
    delete eratSmall_;
    eratSmall_ = NULL;
  }
  if (eratMedium_ != NULL) {
    delete eratMedium_;
    eratMedium_ = NULL;
  }
  if (eratBig_ != NULL) {
    delete eratBig_;
    eratBig_ = NULL;
  }
}

uint32_t SieveOfEratosthenes::getRemainder(uint64_t n) {
  uint32_t remainder = static_cast<uint32_t> (n % NUMBERS_PER_BYTE);
  return (remainder > 1) ? remainder : remainder + NUMBERS_PER_BYTE;
}

void SieveOfEratosthenes::setSieveSize(uint32_t sieveSize) {
  // It makes no sense to use very small sieve sizes, the CPU's L1 or
  // L2 cache size usually performs best
  if (sieveSize < 1024)
    throw std::invalid_argument(
        "SieveOfEratosthenes: sieve size must be >= 1024.");
  if (sieveSize > UINT32_MAX / NUMBERS_PER_BYTE)
    throw std::overflow_error(
        "SieveOfEratosthenes: sieve size must be <= 2^32 / 30.");
  sieveSize_ = sieveSize;
}

void SieveOfEratosthenes::setLowerBound(uint64_t startNumber) {
  lowerBound_ = startNumber - this->getRemainder(startNumber);
  assert(lowerBound_ % NUMBERS_PER_BYTE == 0);
}

void SieveOfEratosthenes::setResetIndex() {
  resetIndex_ = resetSieve_->getResetIndex(lowerBound_);
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
  // correct reset() for numbers <= 31
  if (startNumber_ <= resetSieve_->getEliminateUpTo())
    sieve_[0] = 0xff;

  uint32_t startRemainder = this->getRemainder(startNumber_);
  // eliminates the bits of the first byte representing numbers
  // smaller than startNumber_
  uint32_t i = 0;
  while (i < 8 && bitValues_[i] < startRemainder)
    i++;
  sieve_[0] &= static_cast<uint8_t> (0xff << i);
}

/**
 * Initialize the 3 sieve of Eratosthenes algorithms if needed.
 */
void SieveOfEratosthenes::initEratAlgorithms() {
  assert(settings::SIEVESIZE_FACTOR_ERATSMALL
      <= settings::SIEVESIZE_FACTOR_ERATMEDIUM);
  uint32_t sqrtStop = U32SQRT(stopNumber_);
  uint32_t limit;
  if (resetSieve_->getEliminateUpTo() < sqrtStop) {
    limit = static_cast<uint32_t> (sieveSize_
        * settings::SIEVESIZE_FACTOR_ERATSMALL);
    eratSmall_ = new EratSmall(std::min<uint32_t>(limit, sqrtStop),
        stopNumber_, sieveSize_);
    if (eratSmall_->getLimit() < sqrtStop) {
      limit = static_cast<uint32_t> (sieveSize_
          * settings::SIEVESIZE_FACTOR_ERATMEDIUM);
      eratMedium_ = new EratMedium(std::min<uint32_t>(limit, sqrtStop),
          stopNumber_, sieveSize_);
      if (eratMedium_->getLimit() < sqrtStop)
        eratBig_ = new EratBig(stopNumber_, sieveSize_);
    }
  }
}

/**
 * Use the erat* algorithms to cross-off the multiples of the
 * current sieve round.
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
 * Use the sieve of Eratosthenes to sieve up to primeNumber^2.
 */
void SieveOfEratosthenes::sieve(uint32_t primeNumber) {
  assert(eratSmall_ != NULL
      && primeNumber > resetSieve_->getEliminateUpTo()
      && U64SQUARE(primeNumber) <= stopNumber_);
  /// @remark '- 6' is a correction for primes of type n * 30 + 31
  const uint64_t primeSquared = U64SQUARE(primeNumber) - 6;
  // do not enter this while loop until all primes required for
  // sieving the next round have been added to one of the erat*
  // algorithms below
  while (lowerBound_ + sieveSize_ * NUMBERS_PER_BYTE < primeSquared) {
    this->crossOffMultiples();
    this->analyseSieve(sieve_, sieveSize_);
    resetSieve_->reset(sieve_, sieveSize_, &resetIndex_);
    lowerBound_ += sieveSize_ * NUMBERS_PER_BYTE;
  }
  // add primeNumber to the appropriate (fastest) sieve of
  // Eratosthenes implementation
  if (eratSmall_->getLimit() >= primeNumber) {
    eratSmall_->addPrimeNumber(primeNumber, lowerBound_);
  } else if (eratMedium_->getLimit() >= primeNumber) {
    eratMedium_->addPrimeNumber(primeNumber, lowerBound_);
  } else {
    eratBig_->addPrimeNumber(primeNumber, lowerBound_);
  }
}

/**
 * Sieve the last rounds remaing after that @code sieve(uint32_t)
 * @endcode has been called consecutively for all primes up to 
 * sqrt(stopNumber).
 */
void SieveOfEratosthenes::finish() {
  assert(lowerBound_ < stopNumber_);
  /// sieve all the rounds left except the last one
  /// @remark '+ 1' is a correction for primes of type n * 30 + 31
  while (lowerBound_ + sieveSize_ * NUMBERS_PER_BYTE + 1 < stopNumber_) {
    this->crossOffMultiples();
    this->analyseSieve(sieve_, sieveSize_);
    resetSieve_->reset(sieve_, sieveSize_, &resetIndex_);
    lowerBound_ += sieveSize_ * NUMBERS_PER_BYTE;
  }
  uint32_t stopRemainder = this->getRemainder(stopNumber_);
  // calculate the sieve size of the last sieve round
  sieveSize_ = static_cast<uint32_t> ((stopNumber_ - stopRemainder)
      - lowerBound_) / NUMBERS_PER_BYTE + 1;
  assert(lowerBound_ + (sieveSize_ - 1) * NUMBERS_PER_BYTE + stopRemainder
      == stopNumber_);
  // sieve the last round
  this->crossOffMultiples();
  // eliminates the bits of the last byte representing
  // numbers greater than stopNumber_
  for (uint32_t i = 0; i < 8; i++) {
    if (stopRemainder < bitValues_[i]) {
      sieve_[sieveSize_ - 1] &= 0xff >> (8 - i);
      break;
    }
  }
  this->analyseSieve(sieve_, sieveSize_);
}
