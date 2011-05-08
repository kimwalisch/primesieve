/*
 * PrimeNumberGenerator.h -- This file is part of primesieve
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

#ifndef PRIMENUMBERGENERATOR_H
#define PRIMENUMBERGENERATOR_H

#include "SieveOfEratosthenes.h"
#include <stdint.h>

class PrimeNumberFinder;

/**
 * Sieve of Eratosthenes that is used to generate the prime numbers
 * up to sqrt(stopNumber) needed for sieving by PrimeNumberFinder.
 */
class PrimeNumberGenerator: public SieveOfEratosthenes {
public:
  PrimeNumberGenerator(PrimeNumberFinder*);
  ~PrimeNumberGenerator();
private:
  PrimeNumberFinder* const primeNumberFinder_;
  /**
   * Lookup table used to calculate the prime number corresponding
   * to a given 1 bit of the sieve_ array.
   */
  uint32_t** primeBitValues_;
  void initPrimeBitValues();
  void analyseSieve(const uint8_t*, uint32_t);
};

#endif /* PRIMENUMBERGENERATOR_H */
