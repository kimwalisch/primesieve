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
#include "defs.h"

class PrimeNumberFinder;

/**
 * PrimeNumberGenerator is a SieveOfEratosthenes class that is used
 * to generate the prime numbers up to n^0.5 needed by
 * PrimeNumberFinder to sieve up to n.
 */
class PrimeNumberGenerator: public SieveOfEratosthenes {
public:
  PrimeNumberGenerator(PrimeNumberFinder&);
private:
  PrimeNumberFinder& primeNumberFinder_;
  void generate(const uint8_t*, uint32_t);
  virtual void analyseSieve(const uint8_t*, uint32_t);
};

#endif /* PRIMENUMBERGENERATOR_H */
