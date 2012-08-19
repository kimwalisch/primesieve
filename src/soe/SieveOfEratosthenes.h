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

#ifndef SIEVEOFERATOSTHENES_H
#define SIEVEOFERATOSTHENES_H

#include "config.h"
#include "PreSieve.h"

#include <stdint.h>

namespace soe {

class EratSmall;
class EratMedium;
class EratBig;

/// SieveOfEratosthenes is an implementation of the segmented sieve of
/// Eratosthenes with wheel factorization. It uses a bit array with 30
/// numbers per byte for sieving and 3 different sieve of Eratosthenes
/// algorithms optimized for small, medium and and big sieving primes.
/// Its main method is sieve(uint_t) it must be called consecutively
/// for all primes up to sqrt(n) in order to sieve the primes up to n.
/// SieveOfEratosthenes is an abstract class, PrimeNumberFinder and
/// PrimeNumberGenerator are derived from it.
///
class SieveOfEratosthenes {
public:
  /// SieveOfEratosthenes uses dense bit packing with 30 numbers
  /// per byte. Each byte of the sieve_ array holds the values
  /// i * 30 + k with k = { 7, 11, 13, 17, 19, 23, 29, 31 }, that
  /// is 8 values per byte and thus one for each bit.
  enum { NUMBERS_PER_BYTE = 30 };
  uint64_t getStart() const;
  uint64_t getStop() const;
  uint_t getSqrtStop() const;
  uint_t getSieveSize() const;
  uint_t getPreSieve() const;
  void sieve(uint_t);
  void finish();
protected:
  static const uint_t bitValues_[8];
  static const uint_t bruijnBitValues_[32];
  SieveOfEratosthenes(uint64_t, uint64_t, uint_t, uint_t);
  virtual ~SieveOfEratosthenes();
  virtual void segmentProcessed(const uint8_t*, uint_t) = 0;
  template<typename T>
  T getNextPrime(uint_t, uint_t*) const;
private:
  /// The current segment is [segmentLow_, segmentHigh_]
  uint64_t segmentLow_;
  uint64_t segmentHigh_;
  /// Sieve the primes within [start_, stop_]
  const uint64_t start_;
  const uint64_t stop_;
  /// sqrt(stop_)
  const uint_t sqrtStop_;
  /// Sieve of Eratosthenes array
  uint8_t* sieve_;
  /// Size of the sieve_ array in bytes
  uint_t sieveSize_;
  /// Used to pre-sieve multiples of small primes e.g. <= 19
  const PreSieve preSieve_;
  /// Sieve of Eratosthenes algorithm for small sieving primes
  EratSmall* eratSmall_;
  /// Sieve of Eratosthenes algorithm for medium sieving primes
  EratMedium* eratMedium_;
  /// Sieve of Eratosthenes algorithm for big sieving primes
  EratBig* eratBig_;
  static uint64_t getByteRemainder(uint64_t);
  void initEratAlgorithms();
  void preSieve();
  void crossOffMultiples();
  void sieveSegment();
  /// Uncopyable, declared but not defined
  SieveOfEratosthenes(const SieveOfEratosthenes&);
  SieveOfEratosthenes& operator=(const SieveOfEratosthenes&);
};

} // namespace soe

#endif
