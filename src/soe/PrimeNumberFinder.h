//
// Copyright (c) 2011 Kim Walisch, <kim.walisch@gmail.com>.
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

#ifndef PRIMENUMBERFINDER_H
#define PRIMENUMBERFINDER_H

#include "SieveOfEratosthenes.h"
#include "defs.h"

class PrimeSieve;

/**
 * PrimeNumberFinder is a SieveOfEratosthenes class that is used to
 * generate, count and print primes and prime k-tuplets (twin primes,
 * prime triplets, ...).
 * PrimeNumberFinder objects are used in PrimeSieve::sieve().
 */
class PrimeNumberFinder: public SieveOfEratosthenes {
public:
  PrimeNumberFinder(PrimeSieve&);
  ~PrimeNumberFinder();
  bool needGenerator() const;
private:
  enum { END = 0xFF + 1 };
  static const uint32_t kTupletBitmasks_[6][5];
  /** Reference to the parent PrimeSieve object. */
  PrimeSieve& ps_;
  /**
   * Lookup tables that give the count of prime k-tuplets
   * (twin primes, prime triplets, ...) per byte.
   */
  uint32_t** kTupletByteCounts_;
  void initLookupTables();
  virtual void analyseSieve(const uint8_t*, uint32_t);
  void count(const uint8_t*, uint32_t);
  void generate(const uint8_t*, uint32_t);
  void callback32_OOP(uint32_t) const;
  void callback64_OOP(uint64_t) const;
  void print(uint64_t) const;
};

#endif /* PRIMENUMBERFINDER_H */
