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

#include <stdint.h>
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
 * == USAGE EXAMPLES ==
 *
 * 1. Count the prime numbers up to 4294967295
 * 
 *    PrimeSieve primesieve;
 *    uint64_t primeCount = primesieve.getPrimeCount(2, 4294967295);
 *
 * 2. Use PrimeSieve as a prime number generator, myPrimes(uint64_t)
 *    will be called consecutively for each prime number up to 10000
 *
 *    void myPrimes(uint64_t prime) {
 *      // do something with the prime
 *    }
 *    ...
 *    PrimeSieve primesieve;
 *    primesieve.generatePrimes(2, 10000, myPrimes);
 *
 * 3. Count and print (to std::cout) the twin primes between 1000 and
 *    2000
 *
 *    PrimeSieve primesieve;
 *    primesieve.setStartNumber(1000);
 *    primesieve.setStopNumber(2000);
 *    primesieve.setFlags(COUNT_TWINS | PRINT_TWINS);
 *    primesieve.sieve();
 *    uint64_t twinCount = primesieve.getTwinCount();
 */
class PrimeSieve {
  friend class ParallelPrimeSieve;
  friend class PrimeNumberFinder;
public:
  enum { COUNTS_SIZE = PrimeNumberFinder::COUNTS_SIZE };
  PrimeSieve();
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
  void sieve();
protected:
  /** Lower bound for sieving. */
  uint64_t startNumber_;
  /** Upper bound for sieveing. */
  uint64_t stopNumber_;
  /** Size of the sieve of Eratosthenes (PrimeNumberFinder) in Bytes. */
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
  /** Status of the sieving process in percent. */
  double status_;
  /** Time elapsed in seconds of the last sieve(void) session. */
  double timeElapsed_;
  void reset();
  virtual void doStatus(uint64_t);
private:
  /** Imperative style callback function for use with generatePrimes(). */
  void (*callback_imp)(uint64_t);
  /** Object-oriented programming style callback function for use with generatePrimes(). */
  void (*callback_oop)(uint64_t, void*);
  void* cbObj_;
  /** Either this or the parent ParallelPrimeSieve object. */
  PrimeSieve* parent_;
  /** Sum of the segments that have been sieved (for status_). */
  uint64_t segments_;
  void setChildPrimeSieve(uint64_t, uint64_t, PrimeSieve*);
  void doSmallPrime(uint32_t, uint32_t, uint32_t, std::string);
};

#endif /* PRIMESIEVE_H */
