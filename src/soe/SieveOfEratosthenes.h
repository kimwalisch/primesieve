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

#ifndef SIEVEOFERATOSTHENES_H
#define SIEVEOFERATOSTHENES_H

#include "defs.h"
#include "PreSieve.h"

class EratSmall;
class EratMedium;
class EratBig;

/**
 * SieveOfEratosthenes is an implementation of the segmented sieve of
 * Eratosthenes using a bit array with 30 numbers per byte.
 * Each byte of the sieve array holds the numbers i * 30 + k with
 * k = {7, 11, 13, 17, 19, 23, 29, 31}, this byte arrangement is
 * convenient for prime k-tuplet sieving.
 * The main function is SieveOfEratosthenes::sieve(uint32_t) it must
 * be called consecutively for all prime numbers up to n^0.5 in order
 * to sieve the primes up to n.
 * Each sieving prime is first added to one of the EratSmall,
 * EratMedium or EratBig objects which are used to cross off the
 * multiples using wheel factorization.
 *
 * SieveOfEratosthenes is an abstract class, PrimeNumberGenerator and
 * PrimeNumberFinder are derived from SieveOfEratosthenes.
 */
class SieveOfEratosthenes {
public:
  enum {
    /**
     * SieveOfEratosthenes uses dense bit packing with 30 numbers
     * per byte, each byte of the sieve_ array holds the values
     * i * 30 + k with k = {7, 11, 13, 17, 19, 23, 29, 31}.
     */
    NUMBERS_PER_BYTE = 30
  };
  uint64_t getSegmentLow() const {
    return segmentLow_;
  }
  uint64_t getStartNumber() const {
    return startNumber_;
  }
  uint64_t getStopNumber() const {
    return stopNumber_;
  }
  uint32_t getSquareRoot() const {
    return sqrtStop_;
  }
  uint32_t getSieveSize() const {
    return sieveSize_;
  }
  uint32_t getPreSieveLimit() const;
  void sieve(uint32_t);
  void finish();
protected:
  static const uint32_t bitValues_[32];
  SieveOfEratosthenes(uint64_t, uint64_t, uint32_t, uint32_t);
  ~SieveOfEratosthenes();
  virtual void analyseSieve(const uint8_t*, uint32_t) = 0;
private:
  /** Lower bound of the current segment. */
  uint64_t segmentLow_;
  /** Upper bound of the current segment. */
  uint64_t segmentHigh_;
  /** The start number for sieving. */
  const uint64_t startNumber_;
  /** The stop number for sieving. */
  const uint64_t stopNumber_;
  /** sqrt(stopNumber_) */
  const uint32_t sqrtStop_;
  /** Sieve of Eratosthenes array. */
  uint8_t* sieve_;
  /** Size of sieve_ in bytes. */
  uint32_t sieveSize_;
  /** Set to false when the first segment has been sieved. */
  bool isFirstSegment_;
  /** Pre-sieves multiples of small primes <= preSieve_.getLimit(). */
  const PreSieve preSieve_;
  /**
   * Used to cross off multiples of small sieving primes that have a
   * lot of multiple occurrences per segment.
   */
  EratSmall* eratSmall_;
  /**
   * Used to cross off multiples of medium sieving primes that have a
   * few multiple occurrences per segment.
   */
  EratMedium* eratMedium_;
  /**
   * Used to cross off multiples of big sieving primes that have less
   * than one multiple occurrence per segment.
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

#endif /* SIEVEOFERATOSTHENES_H */
