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
#include "defs.h"
#include "pmath.h"

#include <cstdlib>
#include <cassert>

namespace {
  const uint32_t BYTE_SIZE = (1 << 8);
  const uint32_t END = BYTE_SIZE;
}

PrimeNumberGenerator::PrimeNumberGenerator(PrimeNumberFinder& finder) :
  SieveOfEratosthenes(
      finder.getPreSieveLimit() + 1,
      isqrt(finder.getStopNumber()),
      defs::PRIMENUMBERGENERATOR_SIEVESIZE * 1024,
      defs::PRIMENUMBERGENERATOR_PRESIEVE_LIMIT),
      primeNumberFinder_(finder), primeBitValues_(NULL) {
  assert(this->getStopNumber() <= UINT32_MAX);
  this->initPrimeBitValues();
}

PrimeNumberGenerator::~PrimeNumberGenerator() {
  for (uint32_t i = 0; i < BYTE_SIZE; i++)
    delete[] primeBitValues_[i];
  delete[] primeBitValues_;
}

/**
 * Initialize the primeBitValues_ lookup table.
 * primeBitValues_ is used to reconstruct prime numbers from 1 bits of
 * the sieve array.
 */
void PrimeNumberGenerator::initPrimeBitValues() {
  primeBitValues_ = new uint32_t*[BYTE_SIZE];
  for (uint32_t i = 0; i < BYTE_SIZE; i++) {
    primeBitValues_[i] = new uint32_t[9];
    uint32_t bitCount = 0;
    // save the bit values of the current byte value (i)
    for (uint32_t j = 0; (1u << j) <= i; j++) {
      if ((1u << j) & i)
        primeBitValues_[i][bitCount++] = bitValues_[j];
    }
    primeBitValues_[i][bitCount] = END;
  }
}

/**
 * Generate the prime numbers within the current segment needed for
 * sieving by primeNumberFinder_ (is a SieveOfEratosthenes).
 * @see SieveOfEratosthenes::sieve(uint32_t)
 */
void PrimeNumberGenerator::generate(const uint8_t* sieve, uint32_t sieveSize) {
  uint32_t byteValue = static_cast<uint32_t> (this->getSegmentLow());

  for (uint32_t i = 0; i < sieveSize; i++, byteValue += NUMBERS_PER_BYTE) {
    for (uint32_t* bitValue = primeBitValues_[sieve[i]]; *bitValue != END; bitValue++) {
      uint32_t prime = byteValue + *bitValue;
      primeNumberFinder_.sieve(prime);
    }
  }
}

void PrimeNumberGenerator::analyseSieve(const uint8_t* sieve, uint32_t sieveSize) {
  this->generate(sieve, sieveSize);
}
