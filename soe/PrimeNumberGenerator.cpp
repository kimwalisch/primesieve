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
#include "cpuid.h"

#include <cassert>

PrimeNumberGenerator::PrimeNumberGenerator(PrimeNumberFinder& finder) :
  SieveOfEratosthenes(
      finder.getPreSieveLimit() + 1,
      isqrt(finder.getStopNumber()),
      defs::PRIMENUMBERGENERATOR_SIEVESIZE * 1024,
      defs::PRIMENUMBERGENERATOR_PRESIEVE_LIMIT),
      primeNumberFinder_(finder) {
  assert(this->getStopNumber() <= UINT32_MAX);
}

/**
 * Generate the primes of the current segment (1 bits of the sieve
 * array) and use them to sieve with primeNumberFinder_
 * (is a SieveOfEratosthenes).
 * @see SieveOfEratosthenes::sieve(uint32_t)
 */
void PrimeNumberGenerator::generate(const uint8_t* sieve, uint32_t sieveSize) {
  uint32_t lowerBound = static_cast<uint32_t> (this->getSegmentLow());
  uint32_t i = 0;

  for (; i < sieveSize / sizeof(uint32_t); i++) {
    uint32_t s32 = reinterpret_cast<const uint32_t*> (sieve)[i];
    while (s32 != 0) {
      uint32_t bitPosition = bitScanForward(s32);
      // unset the current bit
      s32 &= s32 - 1;
      uint32_t prime = lowerBound + bitValues_[bitPosition];
      primeNumberFinder_.sieve(prime);
    }
    lowerBound += NUMBERS_PER_BYTE * sizeof(uint32_t);
  }
  // process the remaining bytes (MAX 3)
  for (i *= sizeof(uint32_t); i < sieveSize; i++) {
    uint32_t s = sieve[i];
    while (s != 0) {
      uint32_t bitPosition = bitScanForward(s);
      s &= s - 1;
      uint32_t prime = lowerBound + bitValues_[bitPosition];
      primeNumberFinder_.sieve(prime);
    }
    lowerBound += NUMBERS_PER_BYTE;
  }
}

void PrimeNumberGenerator::analyseSieve(const uint8_t* sieve, uint32_t sieveSize) {
  // the C++ Standard guarantees that memory is suitably aligned, 
  // see "3.7.3.1 Allocation functions"
  assert(reinterpret_cast<uintptr_t> (sieve) % sizeof(uint32_t) == 0);
  this->generate(sieve, sieveSize);
}
