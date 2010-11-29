/*
 * PrimeSieve.cpp -- This file is part of primesieve
 *
 * Copyright (C) 2010 Kim Walisch, <kim.walisch@gmail.com>
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

#include "PrimeSieve.h"
#include "settings.h"
#include "pmath.h"

#include <stdint.h>
#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <cassert>
#include <algorithm>
#include <string>
#include <vector>

PrimeSieve::PrimeSieve() :
  startNumber_(0), stopNumber_(0), sieveSize_(
      settings::DEFAULT_SIEVESIZE_PRIMENUMBERFINDER), flags_(0), results_(
      &primeSieveResults_), resetSieve_(NULL), primeNumberGenerator_(NULL),
      primeNumberFinder_(NULL) {
  // default setting count prime numbers
  this->setFlags(COUNT_PRIMES);
}

PrimeSieve::~PrimeSieve() {
  if (primeNumberGenerator_ != NULL)
    delete primeNumberGenerator_;
  if (primeNumberFinder_ != NULL)
    delete primeNumberFinder_;
  if (resetSieve_ != NULL)
    delete resetSieve_;
}

uint64_t PrimeSieve::getStartNumber() const {
  return startNumber_;
}

uint64_t PrimeSieve::getStopNumber() const {
  return stopNumber_;
}

uint32_t PrimeSieve::getSieveSize() const {
  return sieveSize_;
}

uint32_t PrimeSieve::getFlags() const {
  return flags_;
}

/**
 * @return The count of prime numbers or k-tuplets between
 *         startNumber and stopNumber or -1 if the appropriate
 *         count flag is not set.
 *
 * @param  index 0 = Prime number count
 *               1 = Twin prime count
 *               2 = Prime triplet count
 *               3 = Prime quadruplet count
 *               4 = Prime quintuplet count
 *               5 = Prime sextuplet count
 *               6 = Prime septuplet count
 */
int64_t PrimeSieve::getCount(uint32_t index) const {
  if (index >= results_->COUNTS_SIZE)
    throw std::out_of_range(
        "PrimeSieve: getCount index must be < 7");
  return results_->counts[index];
}

/**
 * Set a start number for sieving.
 * @pre startNumber < (2^64-1) - (2^32-1) * 10
 */
void PrimeSieve::setStartNumber(uint64_t startNumber) {
  // EratBig and EratMedium stopNumber limit
  if (startNumber >= UINT64_MAX - UINT32_MAX * UINT64_C(10))
    throw std::invalid_argument(
        "PrimeSieve: startNumber must be < (2^64-1) - (2^32-1) * 10.");
  startNumber_ = startNumber;
}

/**
 * Set a stop number for sieving.
 * @pre stopNumber < (2^64-1) - (2^32-1) * 10
 */
void PrimeSieve::setStopNumber(uint64_t stopNumber) {
  if (stopNumber < startNumber_)
    throw std::invalid_argument(
        "PrimeSieve: stopNumber must be >= startNumber.");
  // EratBig and EratMedium stopNumber limit
  if (stopNumber >= UINT64_MAX - UINT32_MAX * UINT64_C(10))
    throw std::invalid_argument(
        "PrimeSieve: stopNumber must be < (2^64-1) - (2^32-1) * 10.");
  stopNumber_ = stopNumber;
}

/**
 * Set the sieve size of PrimeSieve (uses the sieve of Eratosthenes).
 * @pre sieveSize >=    1 KiloByte  &&
 *      sieveSize <= 8192 KiloBytes &&
 *      sieveSize must be a power of 2.
 */
void PrimeSieve::setSieveSize(uint32_t sieveSize) {
  // SieveOfEratosthenes lower sieve size limit
  if (sieveSize < 1)
    throw std::invalid_argument("PrimeSieve: sieve size must be >= 1 KiloByte.");
  // EratBig upper sieveSize limit
  if (sieveSize > 8192)
    throw std::invalid_argument(
        "PrimeSieve: sieveSize must be <= 8192 KiloBytes.");
  // EratBig requires a power of 2 sieve size
  if (!isPowerOf2(sieveSize))
    throw std::invalid_argument("PrimeSieve: sieveSize must be a power of 2.");
  // convert to Bytes
  sieveSize_ = sieveSize * 1024;
}

/**
 * Set the flags (settings) of PrimeSieve.
 * @link PrimeNumberFinder.h
 * @param flags
 *   COUNT_PRIMES      OR (bitwise '|')
 *   COUNT_TWINS       OR
 *   COUNT_TRIPLETS    OR
 *   COUNT_QUADRUPLETS OR
 *   COUNT_QUINTUPLETS OR
 *   COUNT_SEXTUPLETS  OR
 *   COUNT_SEPTUPLETS  OR
 *   PRINT_PRIMES      OR
 *   PRINT_TWINS       OR
 *   PRINT_TRIPLETS    OR
 *   PRINT_QUADRUPLETS OR
 *   PRINT_QUINTUPLETS OR
 *   PRINT_SEXTUPLETS  OR
 *   PRINT_SEPTUPLETS  OR
 *   PRINT_STATUS      OR
 *   STORE_STATUS.
 */
void PrimeSieve::setFlags(uint32_t flags) {
  flags_ = flags;
  results_->reset(flags_);
}

/** For use with multi-thread versions of PrimeSieve. */
void PrimeSieve::setResults(PrimeNumberFinder::Results* results) {
  results_ = results;
}

void PrimeSieve::initSieveOfEratosthenes() {
  assert(primeNumberFinder_ == NULL && primeNumberGenerator_ == NULL);
  if (stopNumber_ >= 7) {
    if (resetSieve_ == NULL)
      resetSieve_ = new ResetSieve(settings::PREELIMINATE_RESETSIEVE);
    // sieves the primes between startNumber_ and stopNumber_
    primeNumberFinder_ = new PrimeNumberFinder(
        std::max<uint64_t>(startNumber_, 7),
        stopNumber_,
        sieveSize_,
        resetSieve_,
        results_,
        flags_);
    if (U32SQRT(stopNumber_) > resetSieve_->getEliminateUpTo()) {
      // generates the prime numbers up to sqrt(stopNumber_)
      primeNumberGenerator_ = new PrimeNumberGenerator(
          settings::SIEVESIZE_PRIMENUMBERGENERATOR,
          primeNumberFinder_);
    }
  }
}

/**
 * Sieve the prime numbers and prime k-tuplets between
 * startNumber and startNumber.
 */
void PrimeSieve::sieve() {
  if (flags_ & PRINT_STATUS)
    std::cout << "\r0%" << std::flush;
  results_->reset(flags_);
  // small primes have to be examined manually
  if (startNumber_ <= 5) {
    uint32_t lowerBound[8] = { 2, 3, 5, 3, 5, 5, 5, 5 };
    uint32_t upperBound[8] = { 2, 3, 4, 5, 7, 11, 13, 17 };
    uint32_t type[8] = { 0, 0, 0, 1, 1, 2, 3, 4 };
    std::string text[8] = { "2", "3", "5", "(3, 5)", "(5, 7)", "(5, 7, 11)",
        "(5, 7, 11, 13)", "(5, 7, 11, 13, 17)" };
    for (uint32_t i = 0; i < 8 && stopNumber_ >= upperBound[i]; i++) {
      if (startNumber_ <= lowerBound[i]) {
        if (flags_ & (COUNT_PRIMES << type[i]))
          results_->counts[type[i]]++;
        if (flags_ & (PRINT_PRIMES << type[i]))
          std::cout << text[i] << std::endl;
      }
    }
  }
  this->initSieveOfEratosthenes();
  if (primeNumberGenerator_ != NULL) {
    std::vector < uint32_t > primes16Bit;
    primes16Bit.push_back(3);
    uint32_t stop = U32SQRT(primeNumberGenerator_->getStopNumber());
    uint32_t keep = U32SQRT(stop);
    // The following trial division algorithm is used to generate the
    // prime numbers up to \sqrt[4]{stopNumber_}. Although the
    // algorithm is never used > 65536 it finds the prime numbers up
    // to 10^7 in 1 second on an Intel Core i5-670 3.46GHz.
    for (uint32_t n = 5; n <= stop; n += 2) {
      uint32_t s = U32SQRT(n);
      uint32_t i = 0;
      for (; primes16Bit[i] <= s && (n % primes16Bit[i]) != 0; i++)
        ;
      if (primes16Bit[i] > s) {
        if (primes16Bit[i] <= keep)
          primes16Bit.push_back(n);
        if (n > resetSieve_->getEliminateUpTo())
          // generates the prime numbers up to n^2 and uses them with
          // primeNumberFinder_ to sieve up to n^4
          primeNumberGenerator_->sieve(n);
      }
    }
    // generate the last remaining primes up to sqrt(stopNumber_)
    primeNumberGenerator_->finish();
    delete primeNumberGenerator_;
    primeNumberGenerator_ = NULL;
  }
  // sieve the last remaining rounds
  if (primeNumberFinder_ != NULL) {
    primeNumberFinder_->finish();
    delete primeNumberFinder_;
    primeNumberFinder_ = NULL;
  }
  // sieving finished, set status to 100%
  if (flags_ & PRINT_STATUS)
    std::cout << "\r100%" << std::endl;
  if (flags_ & STORE_STATUS)
    results_->status = 100.0;
}
