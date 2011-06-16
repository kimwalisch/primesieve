/*
 * PrimeNumberFinder.h -- This file is part of primesieve
 *
 * Copyright (C) 2011 Kim Walisch, <kim.walisch@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef PRIMENUMBERFINDER_H
#define PRIMENUMBERFINDER_H

#include "SieveOfEratosthenes.h"
#include "defs.h"

class PrimeSieve;

/**
 * PrimeNumberFinder is a SieveOfEratosthenes class that is used to
 * count, print and generate the prime numbers and prime k-tuplets
 * (twin primes, prime triplets, ...) in the interval
 * [startNumber_, stopNumber_].
 * The prime numbers up to stopNumber_^0.5 needed for sieving are
 * provided by PrimeNumberGenerator.
 * @see PrimeNumberGenerator::generate(const uint8_t*, uint32_t)
 */
class PrimeNumberFinder: public SieveOfEratosthenes {
public:
  enum { COUNTS_SIZE = 7 };
  PrimeNumberFinder(PrimeSieve*, ResetSieve*);
  ~PrimeNumberFinder();
private:
  static const uint32_t nextBitValue_[NUMBERS_PER_BYTE];
  /** Pointer to the parent PrimeSieve object. */
  PrimeSieve* const primeSieve_;
  /**
   * Lookup table that gives the count of prime numbers and k-tuplets
   * within a byte of the sieve array.
   */
  uint32_t** primeByteCounts_;
  /**
   * Lookup table used to reconstruct prime numbers and k-tuplets
   * from 1 bits or bit patterns of the sieve array.
   */
  uint32_t** primeBitValues_;
  void initLookupTables();
  void count(const uint8_t*, uint32_t);
  void generate(const uint8_t*, uint32_t);
  void analyseSieve(const uint8_t*, uint32_t);
};

#endif /* PRIMENUMBERFINDER_H */
