/*
 * PrimeSieve.h -- This file is part of primesieve
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

#ifndef PRIMESIEVE_H
#define PRIMESIEVE_H

#include "ResetSieve.h"
#include "PrimeNumberFinder.h"
#include "PrimeNumberGenerator.h"

#include <stdint.h>

/**
 * PrimeSieve is a highly optimized implementation of the sieve of
 * Eratosthenes that finds prime numbers and prime k-tuplets
 * (twin primes, prime triplets, ...) up to 2^64.
 */
class PrimeSieve {
public:
  PrimeSieve();
  ~PrimeSieve();
  uint64_t getStartNumber() const;
  uint64_t getStopNumber() const;
  uint32_t getSieveSize() const;
  uint32_t getFlags() const;
   int64_t getCounts(uint32_t) const;
  void setStartNumber(uint64_t);
  void setStopNumber(uint64_t);
  void setSieveSize(uint32_t);
  void setFlags(uint32_t);
  void setResults(PrimeNumberFinder::Results*);
  void sieve();
private:
  /** Lower bound for sieving. */
  uint64_t startNumber_;
  /** Upper bound for sieveing. */
  uint64_t stopNumber_;
  /** Size of the sieve of Eratosthenes. */
  uint32_t sieveSize_;
  /** Settings for PrimeNumberFinder. */
  uint32_t flags_;
  /** The results of sieve() are saved in here. */
  PrimeNumberFinder::Results* results_;
  PrimeNumberFinder::Results primeSieveResults_;
  /** Used to reset the sieve_ array of SieveOfEratosthenes objects. */
  ResetSieve* resetSieve_;
  /** 
   * Sieve of Eratosthenes that generates the prime numbers up to
   * sqrt(stopNumber_) needed for sieving by primeNumberFinder_.
   */
  PrimeNumberGenerator* primeNumberGenerator_;
  /** 
   * Main sieve of Eratosthenes, used to find the prime numbers and
   * prime k-tuplets between startNumber_ and stopNumber_.
   */
  PrimeNumberFinder* primeNumberFinder_;
  void initSieveOfEratosthenes();
};

#endif /* PRIMESIEVE_H */
