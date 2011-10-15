//
// Copyright (c) 2011 Kim Walisch, <kim.walisch@gmail.com>.
// All rights reserved.
//
// This file is part of primesieve.
// Visit: http://primesieve.googlecode.com
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//   * Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above
//     copyright notice, this list of conditions and the following
//     disclaimer in the documentation and/or other materials provided
//     with the distribution.
//   * Neither the name of the author nor the names of its
//     contributors may be used to endorse or promote products derived
//     from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef PRIMESIEVE_H
#define PRIMESIEVE_H

#include "PrimeNumberFinder.h"
#include "defs.h"

#include <string>

class ParallelPrimeSieve;

/**
 * PrimeSieve is a highly optimized implementation of the segmented
 * sieve of Eratosthenes that generates primes and prime k-tuplets
 * (twins, triplets, ...) in order up to 2^64 maximum.
 * The file ../README describes the algorithms used in more detail.
 * The file ../docs/USAGE_EXAMPLES contains source code examples of
 * how to use PrimeSieve & ParallelPrimeSieve objects.
 */
class PrimeSieve {
  friend class PrimeNumberFinder;
public:
  /** Public user flags. */
  enum {
    COUNTS_SIZE       = 7,
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
  /** Private internal falgs (bits >= 20). */
  enum {
    CALLBACK_PRIMES     = 1 << 20,
    CALLBACK_PRIMES_OOP = 1 << 21,
    GENERATE_FLAGS      = PRINT_FLAGS | CALLBACK_PRIMES | CALLBACK_PRIMES_OOP
  };
  /** Sieve the primes within the interval [startNumber_, stopNumber_]. */
  uint64_t startNumber_;
  /** Sieve the primes within the interval [startNumber_, stopNumber_]. */
  uint64_t stopNumber_;
  /** A sieve size in kilobytes. */
  uint32_t sieveSize_;
  /** Settings for PrimeSieve::sieve(). */
  uint32_t flags_;
  /**
   * counts_[0]: count of prime numbers,
   * counts_[1]: count of twin primes,
   * counts_[2]: count of prime triplets,
   * counts_[3]: count of prime quadruplets,
   * counts_[4]: count of prime quintuplets,
   * counts_[5]: count of prime sextuplets,
   * counts_[6]: count of prime septuplets.
   */
  uint64_t counts_[COUNTS_SIZE];
  /** Status in percent of PrimeSieve::sieve(). */
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
