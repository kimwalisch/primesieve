/*
 * PrimeNumberGenerator.cpp -- This file is part of primesieve
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

#include "PrimeNumberGenerator.h"
#include "PrimeNumberFinder.h"
#include "SieveOfEratosthenes.h"
#include "ResetSieve.h"
#include "defs.h"
#include "pmath.h"

#include <cstdlib>
#include <cassert>

namespace {
  const uint32_t BYTE_SIZE = 256;
  const uint32_t END = 0xff;
}

PrimeNumberGenerator::PrimeNumberGenerator(PrimeNumberFinder* primeNumberFinder) :
  SieveOfEratosthenes(
      primeNumberFinder->getResetSieve()->getLimit() + 1,
      isqrt(primeNumberFinder->getStopNumber()),
      defs::SIEVESIZE_PRIMENUMBERGENERATOR,
      primeNumberFinder->getResetSieve()),
      primeNumberFinder_(primeNumberFinder), primeBitValues_(NULL) {
  // PrimeNumberGenerator uses 32-bit integers
  assert(this->getStopNumber() <= UINT32_MAX);
  this->initPrimeBitValues();
}

PrimeNumberGenerator::~PrimeNumberGenerator() {
  for (uint32_t i = 0; i < BYTE_SIZE; i++)
    delete[] primeBitValues_[i];
  delete[] primeBitValues_;
}

/**
 * Create a lookup table with the bitValues_ of the 256 possible
 * values of a byte.
 */
void PrimeNumberGenerator::initPrimeBitValues() {
  primeBitValues_ = new uint32_t*[BYTE_SIZE];
  // generate the bitValues for each byte value
  for (uint32_t i = 0; i < BYTE_SIZE; i++) {
    primeBitValues_[i] = new uint32_t[9];
    uint32_t bitCount = 0;
    // save the bitValues of the current byte value
    for (uint32_t j = 0; (1u << j) <= i; j++) {
      if ((1u << j) & i) {
        primeBitValues_[i][bitCount] = bitValues_[j];
        bitCount++;
      }
    }
    primeBitValues_[i][bitCount] = END;
  }
}

/**
 * Generate the prime numbers of the current segment and use them to
 * sieve with primeNumberFinder_ (is a SieveOfEratosthenes).
 * @see SieveOfEratosthenes::sieve(uint32_t)
 */
void PrimeNumberGenerator::generate(const uint8_t* sieve, uint32_t sieveSize) {
  uint32_t byteValue = static_cast<uint32_t> (this->getLowerBound());
  for (uint32_t i = 0; i < sieveSize; i++) {
    // generate the prime numbers within the current byte
    for (uint32_t* bitValue = primeBitValues_[sieve[i]]; *bitValue != END; bitValue++)
      primeNumberFinder_->sieve(byteValue + *bitValue);
    byteValue += NUMBERS_PER_BYTE;
  }
}

void PrimeNumberGenerator::analyseSieve(const uint8_t* sieve, uint32_t sieveSize) {
  this->generate(sieve, sieveSize);
}
