//
// Copyright (c) 2012 Kim Walisch, <kim.walisch@gmail.com>.
// All rights reserved.
//
// This file is part of primesieve.
// Homepage: http://primesieve.googlecode.com
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
#include "config.h"

#include <stdint.h>
#include <stdexcept>
#include <string>

/**
 * PrimeSieve is a highly optimized implementation of the segmented
 * sieve of Eratosthenes that generates primes and prime k-tuplets
 * (twins, triplets, ...) in order up to 2^64 maximum.
 * The file /README describes the algorithms used in more detail and
 * /docs/USAGE_EXAMPLES contains source code examples that show how to
 * use PrimeSieve objects.
 */
class PrimeSieve {
  friend class soe::PrimeNumberFinder;
public:
  class cancel_sieving : public std::runtime_error {
  public:
    cancel_sieving() : 
      std::runtime_error("PrimeSieve: sieving canceled.") { }
  };
  enum { COUNTS_SIZE = 7 };
  /** Public flags for sieve(). */
  enum {
    COUNT_PRIMES      = 1 << 0,
    COUNT_TWINS       = 1 << 1,
    COUNT_TRIPLETS    = 1 << 2,
    COUNT_QUADRUPLETS = 1 << 3,
    COUNT_QUINTUPLETS = 1 << 4,
    COUNT_SEXTUPLETS  = 1 << 5,
    COUNT_SEPTUPLETS  = 1 << 6,
    COUNT_KTUPLETS    = COUNT_SEPTUPLETS * 2 - 1 - (COUNT_TWINS - 1),
    COUNT_FLAGS       = COUNT_PRIMES + COUNT_KTUPLETS,
    PRINT_PRIMES      = 1 << 7,
    PRINT_TWINS       = 1 << 8,
    PRINT_TRIPLETS    = 1 << 9,
    PRINT_QUADRUPLETS = 1 << 10,
    PRINT_QUINTUPLETS = 1 << 11,
    PRINT_SEXTUPLETS  = 1 << 12,
    PRINT_SEPTUPLETS  = 1 << 13,
    PRINT_KTUPLETS    = PRINT_SEPTUPLETS * 2 - 1 - (PRINT_TWINS - 1),
    CALCULATE_STATUS  = 1 << 14,
    PRINT_STATUS      =(1 << 15) + CALCULATE_STATUS
  };
  PrimeSieve();
  PrimeSieve(PrimeSieve*);
  virtual ~PrimeSieve() { }

  /** sieve() parameter getters. */
  uint64_t getStart() const;
  uint64_t getStop() const;
  uint32_t getPreSieveLimit() const;
  uint32_t getSieveSize() const;
  double getTimeElapsed() const;
  double getStatus() const;
  uint32_t getFlags() const;
  bool testFlags(uint32_t) const;
  bool isFlag(uint32_t) const;

  /** sieve() parameter setters. */
  void setStart(uint64_t);
  void setStop(uint64_t);
  void setPreSieveLimit(uint32_t);
  void setSieveSize(uint32_t);
  void setFlags(uint32_t);
  void addFlags(uint32_t);

  /** Sieving member functions. */
  void sieve(uint64_t, uint64_t);
  void sieve(uint64_t, uint64_t, uint32_t);
  virtual void sieve();

  /** Prime number generation methods. */
  void generatePrimes(uint32_t, uint32_t, void (*callback)(uint32_t));
  void generatePrimes(uint32_t, uint32_t, void (*callback)(uint32_t, void*), void* cbObj);
  void generatePrimes(uint64_t, uint64_t, void (*callback)(uint64_t));
  void generatePrimes(uint64_t, uint64_t, void (*callback)(uint64_t, void*), void* cbObj);

  /** Count member functions. */
  uint64_t getPrimeCount(uint64_t, uint64_t);
  uint64_t getTwinCount(uint64_t, uint64_t);
  uint64_t getTripletCount(uint64_t, uint64_t);
  uint64_t getQuadrupletCount(uint64_t, uint64_t);
  uint64_t getQuintupletCount(uint64_t, uint64_t);
  uint64_t getSextupletCount(uint64_t, uint64_t);
  uint64_t getSeptupletCount(uint64_t, uint64_t);

  /** Count member functions after sieve() execution. */
  uint64_t getPrimeCount() const;
  uint64_t getTwinCount() const;
  uint64_t getTripletCount() const;
  uint64_t getQuadrupletCount() const;
  uint64_t getQuintupletCount() const;
  uint64_t getSextupletCount() const;
  uint64_t getSeptupletCount() const;
  uint64_t getCounts(uint32_t) const;

  /** Print member functions (to std::cout). */
  void printPrimes(uint64_t, uint64_t);
  void printTwins(uint64_t, uint64_t);
  void printTriplets(uint64_t, uint64_t);
  void printQuadruplets(uint64_t, uint64_t);
  void printQuintuplets(uint64_t, uint64_t);
  void printSextuplets(uint64_t, uint64_t);
  void printSeptuplets(uint64_t, uint64_t);

  /** Old API (version <= 3.4) keps backward compatibility. */
  uint64_t getStartNumber() const;
  uint64_t getStopNumber() const;
  void setStartNumber(uint64_t);
  void setStopNumber(uint64_t);
protected:
  /** Internal PrimeSieve flags (bits >= 20). */
  enum {
    CALLBACK32_PRIMES     = 1 << 20,
    CALLBACK32_OOP_PRIMES = 1 << 21,
    CALLBACK64_PRIMES     = 1 << 22,
    CALLBACK64_OOP_PRIMES = 1 << 23,
    CALLBACK_FLAGS        = CALLBACK64_OOP_PRIMES * 2 - 1 - (CALLBACK32_PRIMES - 1),
    GENERATE_FLAGS        = CALLBACK_FLAGS + PRINT_PRIMES + PRINT_KTUPLETS
  };
  /** Sieve the primes within [start_, stop_]. */
  uint64_t start_;
  /** Sieve the primes within [start_, stop_]. */
  uint64_t stop_;
  /** Prime number and prime k-tuplet counts. */
  uint64_t counts_[COUNTS_SIZE];
  /** Time elapsed in seconds of sieve(). */
  double timeElapsed_;
  void reset();
  virtual void calcStatus(uint32_t);
  virtual void set_lock();
  virtual void unset_lock();
private:
  class LockGuard {
  public:
    LockGuard(PrimeSieve& ps) : ps_(ps) { ps_.set_lock(); }
    ~LockGuard() { ps_.unset_lock(); }
  private:
    PrimeSieve& ps_;
    LockGuard(const LockGuard&);
    LockGuard& operator=(const LockGuard&);
  };
  /** Multiples of small primes <= preSieveLimit_ are pre-sieved. */
  uint32_t preSieveLimit_;
  /** Sieve size in kilobytes. */
  uint32_t sieveSize_;
  /** PrimeSieve options (e.g. COUNT_PRIMES). */
  uint32_t flags_;
  /** Either NULL or the parent ParallelPrimeSieve object. */
  PrimeSieve* parent_;
  /** Sum of the processed segments. */
  uint64_t sumSegments_;
  /** stop_ - start_ (+ 1 to avoid /0). */
  double interval_;
  /** Status in percent of sieve(). */
  double status_;
  /** Callback functions for use with generatePrimes(). */
  void (*callback32_)(uint32_t);
  void (*callback32_OOP_)(uint32_t, void*);
  void (*callback64_)(uint64_t);
  void (*callback64_OOP_)(uint64_t, void*);
  void* cbObj_;
  void doSmallPrime(uint32_t, uint32_t, uint32_t, const std::string&);
};

#endif /* PRIMESIEVE_H */
