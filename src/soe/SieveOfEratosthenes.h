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

/**
 * SieveOfEratosthenes is an implementation of the segmented sieve of
 * Eratosthenes with wheel factorization.
 * It uses a bit array with 30 numbers per byte and 3 different sieve
 * of Eratosthenes algorithms i.e. Erat(Small|Medium|Big) objects
 * optimized for small, medium and big sieving primes.
 * Its main method is sieve(uint32_t) it must be called consecutively
 * for all primes up to sqrt(n) in order to sieve the primes up to n.
 * @remark SieveOfEratosthenes is an abstract class, PrimeNumberFinder
 *         and PrimeNumberGenerator are derived from it.
 */
class SieveOfEratosthenes {
public:
  enum {
    /**
     * SieveOfEratosthenes uses dense bit packing with 30 numbers
     * per byte. Each byte of the sieve_ array holds the values
     * i * 30 + k with k = {7, 11, 13, 17, 19, 23, 29, 31}, that is
     * 8 values per byte and thus one for each bit.
     */
    NUMBERS_PER_BYTE = 30
  };
  uint64_t getSegmentLow() const {
    return segmentLow_;
  }
  uint64_t getStart() const {
    return start_;
  }
  uint64_t getStop() const {
    return stop_;
  }
  uint32_t getSquareRoot() const {
    return sqrtStop_;
  }
  /** Get the sieve size in bytes. */
  uint32_t getSieveSize() const {
    return sieveSize_;
  }
  uint32_t getPreSieveLimit() const;
  void sieve(uint32_t);
  void finish();
protected:
  static const uint32_t bitValues_[8];
  static const uint32_t bruijnBitValues_[32];
  SieveOfEratosthenes(uint64_t, uint64_t, uint32_t, uint32_t);
  ~SieveOfEratosthenes();
  static uint32_t getFirstSetBitValue(uint32_t);
  virtual void segmentProcessed(const uint8_t*, uint32_t) = 0;
private:
  /** Lower bound of the current segment. */
  uint64_t segmentLow_;
  /** Upper bound of the current segment. */
  uint64_t segmentHigh_;
  /** Sieve the primes within the interval [start_, stop_]. */
  const uint64_t start_;
  /** Sieve the primes within the interval [start_, stop_]. */
  const uint64_t stop_;
  /** sqrt(stop_) */
  const uint32_t sqrtStop_;
  /** Pre-sieves multiples of small primes <= preSieve_.getLimit(). */
  const PreSieve preSieve_;
  /** Set to false when the first segment has been sieved. */
  bool isFirstSegment_;
  /** Sieve of Eratosthenes array. */
  uint8_t* sieve_;
  /** Size of the sieve_ array in bytes. */
  uint32_t sieveSize_;
  /**
   * Used to cross-off the multiples of small sieving primes
   * that have many multiples per segment.
   */
  EratSmall* eratSmall_;
  /**
   * Used to cross-off the multiples of medium sieving primes
   * that have a few multiples per segment.
   */
  EratMedium* eratMedium_;
  /**
   * Used to cross-off the multiples of big sieving primes
   * that have very few multiples per segment.
   */
  EratBig* eratBig_;
  uint32_t getByteRemainder(uint64_t) const;
  void initEratAlgorithms();
  void preSieve();
  void crossOffMultiples();
  /** Uncopyable, declared but not defined. */
  SieveOfEratosthenes(const SieveOfEratosthenes&);
  SieveOfEratosthenes& operator=(const SieveOfEratosthenes&);
};

} // namespace soe

/**
 * Reconstruct prime numbers from 1 bits of the sieve array
 * and call a callback function for each prime.
 * @see PrimeNumberFinder.cpp, PrimeNumberGenerator.cpp
 */
#define GENERATE_PRIMES(callback, uintXX_t) {               \
  uintXX_t offset = static_cast<uintXX_t>(getSegmentLow()); \
  uint32_t i = 0;                                           \
  for (; i < sieveSize - sieveSize % 4; i += 4) {           \
    /* big-endian safe, reinterpret_cast won't work */      \
    uint32_t dword = (sieve[i+0] << (8 * 0)) +              \
                     (sieve[i+1] << (8 * 1)) +              \
                     (sieve[i+2] << (8 * 2)) +              \
                     (sieve[i+3] << (8 * 3));               \
    while (dword != 0) {                                    \
      uintXX_t prime = offset + getFirstSetBitValue(dword); \
      dword &= dword - 1;                                   \
      callback (prime);                                     \
    }                                                       \
    offset += NUMBERS_PER_BYTE * 4;                         \
  }                                                         \
  for (; i < sieveSize; i++, offset += NUMBERS_PER_BYTE) {  \
    uint32_t byte = sieve[i];                               \
    while (byte != 0) {                                     \
      uintXX_t prime = offset + getFirstSetBitValue(byte);  \
      byte &= byte - 1;                                     \
      callback (prime);                                     \
    }                                                       \
  }                                                         \
}

#endif /* SIEVEOFERATOSTHENES_H */
