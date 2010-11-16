/*
 * PrimeNumberGenerator.cpp -- This file is part of primesieve
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

#include "PrimeNumberGenerator.h"

#include <cstdlib>
#include <cassert>

namespace {
  const uint32_t END = ~0u;
  const uint32_t BYTE_SIZE = 256;
}

PrimeNumberGenerator::PrimeNumberGenerator(uint32_t sieveSize,
    PrimeNumberFinder* primeNumberFinder) :
  SieveOfEratosthenes(primeNumberFinder->getResetSieve()->getEliminateUpTo()
      + 1, U32SQRT(primeNumberFinder->getStopNumber()), sieveSize,
      primeNumberFinder->getResetSieve()),
      primeNumberFinder_(primeNumberFinder), primeBitValues_(NULL) {
  this->initPrimeBitValues();
}

PrimeNumberGenerator::~PrimeNumberGenerator() {
  if (primeBitValues_ != NULL) {
    for (uint32_t i = 0; i < BYTE_SIZE; i++) {
      delete[] primeBitValues_[i];
    }
    delete[] primeBitValues_;
  }
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
 * Generate the prime numbers of the current sieve round and use
 * them to sieve with primeNumberFinder_.
 */
void PrimeNumberGenerator::analyseSieve(const uint8_t* sieve,
    uint32_t sieveSize) {
  assert(this->getLowerBound() + sieveSize * NUMBERS_PER_BYTE <= UINT32_MAX);
  uint32_t byteValue = static_cast<uint32_t> (this->getLowerBound());
  for (uint32_t i = 0; i < sieveSize; i++) {
    // generate the prime numbers within the current byte
    for (uint32_t* bitValue = primeBitValues_[sieve[i]]; *bitValue != END; bitValue++) {
      uint32_t primeNumber = byteValue + *bitValue;
      primeNumberFinder_->sieve(primeNumber);
    }
    byteValue += NUMBERS_PER_BYTE;
  }
}
