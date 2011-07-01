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
 * Eratosthenes that generates prime numbers and prime k-tuplets (twin
 * primes, prime triplets, ...) in order up to 2^64 maximum.
 * The file ../README describes the algorithms used in more detail.
 * 
 * == Usage ==
 *
 * The file ../docs/USAGE_EXAMPLES contains source code examples that
 * show how to use PrimeSieve objects to generate primes, count
 * primes, print prime triplets, ...
 *
 * == Memory Requirement ==
 *
 * PrimeSieve::sieve() uses about:
 * pi(stopNumber_^0.5) * 8 Bytes + sieve size + 450 Kilobytes
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
    COUNT_KTUPLETS    = COUNT_TWINS | COUNT_TRIPLETS | COUNT_QUADRUPLETS | COUNT_QUINTUPLETS | COUNT_SEXTUPLETS | COUNT_SEPTUPLETS,
    COUNT_FLAGS       = COUNT_PRIMES | COUNT_KTUPLETS,
    PRINT_PRIMES      = 1 << 7,
    PRINT_TWINS       = 1 << 8,
    PRINT_TRIPLETS    = 1 << 9,
    PRINT_QUADRUPLETS = 1 << 10,
    PRINT_QUINTUPLETS = 1 << 11,
    PRINT_SEXTUPLETS  = 1 << 12,
    PRINT_SEPTUPLETS  = 1 << 13,
    PRINT_KTUPLETS    = PRINT_TWINS | PRINT_TRIPLETS | PRINT_QUADRUPLETS | PRINT_QUINTUPLETS | PRINT_SEXTUPLETS | PRINT_SEPTUPLETS,
    PRINT_FLAGS       = PRINT_PRIMES | PRINT_KTUPLETS,
    PRINT_STATUS      = 1 << 14
  };
  enum {
    COUNTS_SIZE = 7
  };
  PrimeSieve();
  PrimeSieve(uint64_t, uint64_t, ParallelPrimeSieve*);
  virtual ~PrimeSieve() {}
  uint64_t getStartNumber() const;
  uint64_t getStopNumber() const;
  uint32_t getSieveSize() const;
  uint32_t getPreSieveLimit() const;
  uint32_t getFlags() const;
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
  void setPreSieveLimit(uint32_t);
  void setFlags(uint32_t);
  void generatePrimes(uint64_t, uint64_t, void (*)(uint64_t));
  void generatePrimes(uint64_t, uint64_t, void (*)(uint64_t, void*), void*);
  virtual void sieve();
protected:
  /** Private flags (bits >= 20) for PrimeSieve. */
  enum {
    CALLBACK_PRIMES     = 1 << 20,
    CALLBACK_PRIMES_OOP = 1 << 21,
    GENERATE_FLAGS      = PRINT_FLAGS | CALLBACK_PRIMES | CALLBACK_PRIMES_OOP,
    SSE4_POPCNT         = 1 << 24
  };
  /** Start number for sieving. */
  uint64_t startNumber_;
  /** Stop number for sieveing. */
  uint64_t stopNumber_;
  /** A sieve size in Kilobytes. */
  uint32_t sieveSize_;
  /** Settings for PrimeSieve::sieve(). */
  uint32_t flags_;
  /**
   * counts_[0]: count of prime numbers
   * counts_[1]: count of twin primes
   * counts_[2]: count of prime triplets
   * counts_[3]: count of prime quadruplets
   * counts_[4]: count of prime quintuplets
   * counts_[5]: count of prime sextuplets
   * counts_[6]: count of prime septuplets
   */
  uint64_t counts_[COUNTS_SIZE];
  /** Status of PrimeSieve::sieve() in percent. */
  double status_;
  /** Time elapsed in seconds of PrimeSieve::sieve(). */
  double timeElapsed_;
  void reset();
  virtual void doStatus(uint32_t);
private:
  /** Multiples of small primes <= preSieveLimit_ are pre-sieved. */
  uint32_t preSieveLimit_;
  /** Callback function for use with PrimeSieve::generatePrimes(). */
  void (*callback_)(uint64_t);
  /** OOP callback function for use with PrimeSieve::generatePrimes(). */
  void (*callbackOOP_)(uint64_t, void*);
  void* cbObj_;
  /** Either this or the parent ParallelPrimeSieve object. */
  PrimeSieve* parent_;
  /** Sum of the processed segments. */
  uint64_t segments_;
  void doSmallPrime(uint32_t, uint32_t, uint32_t, const std::string&);
};

#endif /* PRIMESIEVE_H */
