/*
 * PrimeSieve.h -- This file is part of primesieve
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

#ifndef PRIMESIEVE_H
#define PRIMESIEVE_H

#include "PrimeNumberFinder.h"
#include "defs.h"

#include <string>

class ParallelPrimeSieve;

/**
 * PrimeSieve is an optimized implementation of the segmented sieve of
 * Eratosthenes that finds prime numbers and prime k-tuplets (twin
 * primes, prime triplets, ...) up to 2^64 maximum.
 *
 * Its algorithm has a complexity of O(n) operations and uses O(n^0.5)
 * space. The memory requirement is 8 bytes per sieving prime i.e.
 * PrimeSieve needs pi(n^0.5)*8 bytes of RAM to sieve up to n (1.6 GB
 * near 2^64).
 *
 * The file ../docs/USAGE_EXAMPLES has source code examples that show
 * how to use PrimeSieve to generate prime numbers, count prime
 * numbers, print twin primes, ...
 */
class PrimeSieve {
  friend class PrimeNumberFinder;
public:
  /** Public flags (settings) for PrimeSieve. */
  enum {
    COUNT_PRIMES      = 1 << 0,
    COUNT_TWINS       = 1 << 1,
    COUNT_TRIPLETS    = 1 << 2,
    COUNT_QUADRUPLETS = 1 << 3,
    COUNT_QUINTUPLETS = 1 << 4,
    COUNT_SEXTUPLETS  = 1 << 5,
    COUNT_SEPTUPLETS  = 1 << 6,
    COUNT_FLAGS       = COUNT_PRIMES | COUNT_TWINS | COUNT_TRIPLETS | COUNT_QUADRUPLETS | COUNT_QUINTUPLETS | COUNT_SEXTUPLETS | COUNT_SEPTUPLETS,
    PRINT_PRIMES      = 1 << 7,
    PRINT_TWINS       = 1 << 8,
    PRINT_TRIPLETS    = 1 << 9,
    PRINT_QUADRUPLETS = 1 << 10,
    PRINT_QUINTUPLETS = 1 << 11,
    PRINT_SEXTUPLETS  = 1 << 12,
    PRINT_SEPTUPLETS  = 1 << 13,
    PRINT_FLAGS       = PRINT_PRIMES | PRINT_TWINS | PRINT_TRIPLETS | PRINT_QUADRUPLETS | PRINT_QUINTUPLETS | PRINT_SEXTUPLETS | PRINT_SEPTUPLETS,
    PRINT_STATUS      = 1 << 14
  };
  enum {
    COUNTS_SIZE = PrimeNumberFinder::COUNTS_SIZE
  };
  PrimeSieve();
  PrimeSieve(uint64_t, uint64_t, ParallelPrimeSieve*);
  virtual ~PrimeSieve() { }
  uint64_t getStartNumber() const;
  uint64_t getStopNumber() const;
  uint32_t getSieveSize() const;
  uint32_t getFlags() const;
  void generatePrimes(uint64_t, uint64_t, void (*)(uint64_t));
  void generatePrimes(uint64_t, uint64_t, void (*)(uint64_t, void*), void*);
  uint64_t getPrimeCount(uint64_t, uint64_t);
  uint64_t getPrimeCount() const;
  uint64_t getTwinCount() const;
  uint64_t getTripletCount() const;
  uint64_t getQuadrupletCount() const;
  uint64_t getQuintupletCount() const;
  uint64_t getSextupletCount() const;
  uint64_t getSeptupletCount() const;
  uint64_t getCounts(uint32_t) const;
  double getTimeElapsed() const;
  void setStartNumber(uint64_t);
  void setStopNumber(uint64_t);
  void setSieveSize(uint32_t);
  void setFlags(uint32_t);
  virtual void sieve();
protected:
  /** Private flags (bits >= 20) for PrimeSieve. */
  enum {
    CALLBACK_PRIMES     = 1 << 20,
    CALLBACK_PRIMES_OOP = 1 << 21,
    GENERATE_FLAGS      = PRINT_FLAGS | CALLBACK_PRIMES | CALLBACK_PRIMES_OOP,
    SSE4_POPCNT         = 1 << 24
  };
  /** Lower bound for sieving. */
  uint64_t startNumber_;
  /** Upper bound for sieveing. */
  uint64_t stopNumber_;
  /** Size of PrimeNumberFinder's sieve array in Bytes. */
  uint32_t sieveSize_;
  /** Flags (settings) for PrimeSieve. */
  uint32_t flags_;
  /**
   * Count of prime numbers    (counts_[0]),
   * Count of twin primes      (counts_[1]),
   * ...,
   * Count of prime septuplets (counts_[6]).
   */
  uint64_t counts_[COUNTS_SIZE];
  /** Status of the sieving process in percent. */
  double status_;
  /** Time elapsed in seconds of the last sieve() call. */
  double timeElapsed_;
  void reset();
  virtual void doStatus(uint32_t);
private:
  /** Callback function for use with generatePrimes(). */
  void (*callback_)(uint64_t);
  /** OOP style callback function for use with generatePrimes(). */
  void (*callbackOOP_)(uint64_t, void*);
  void* cbObj_;
  /** Either this or the parent ParallelPrimeSieve object. */
  PrimeSieve* parent_;
  /** Sum of the segments that have been sieved so far. */
  uint64_t segments_;
  void doSmallPrime(uint32_t, uint32_t, uint32_t, const std::string&);
};

#endif /* PRIMESIEVE_H */
