/*
 * PrimeSieve.h -- This file is part of primesieve
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

#ifndef PRIMESIEVE_H
#define PRIMESIEVE_H

#include "PrimeNumberFinder.h"
#include <stdint.h>

/**
 * PrimeSieve is a highly optimized implementation of the sieve of
 * Eratosthenes that finds prime numbers and prime k-tuplets
 * (twin primes, prime triplets, ...) up to 2^64.
 */
class PrimeSieve {
public:
  enum { COUNTS_SIZE = PrimeNumberFinder::COUNTS_SIZE };
  PrimeSieve();
  uint64_t getStartNumber() const;
  uint64_t getStopNumber() const;
  uint32_t getSieveSize() const;
  uint32_t getFlags() const;
  uint64_t getCounts(uint32_t) const;
  uint64_t getPrimeCount() const;
  uint64_t getTwinCount() const;
  uint64_t getTripletCount() const;
  uint64_t getQuadrupletCount() const;
  uint64_t getQuintupletCount() const;
  uint64_t getSextupletCount() const;
  uint64_t getSeptupletCount() const;
  double getTimeElapsed() const;
  void setStartNumber(uint64_t);
  void setStopNumber(uint64_t);
  void setSieveSize(uint32_t);
  void setFlags(uint32_t);
  void sieve();
protected:
  /** Lower bound for sieving. */
  uint64_t startNumber_;
  /** Upper bound for sieveing. */
  uint64_t stopNumber_;
  /** Size of the sieve of Eratosthenes. */
  uint32_t sieveSize_;
  /** Settings for PrimeNumberFinder. */
  uint32_t flags_;
  /**
    * counts_[0] = prime number count
    * counts_[1] = twin prime count
    * counts_[2] = prime triplet count
    * ...
    * counts_[6] = prime septuplet count
    */
  uint64_t counts_[COUNTS_SIZE];
  /** Time elapsed in seconds of the last sieve(void) session. */
  double timeElapsed_;
};

#endif /* PRIMESIEVE_H */
