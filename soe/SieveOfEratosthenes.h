/*
 * SieveOfEratosthenes.h -- This file is part of primesieve
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

#ifndef SIEVEOFERATOSTHENES_H
#define SIEVEOFERATOSTHENES_H

#include "ResetSieve.h"
#include "EratSmall.h"
#include "EratMedium.h"
#include "EratBig.h"
#include "defs.h"

/**
 * SieveOfEratosthenes is an implementation of the segmented sieve of
 * Eratosthenes using a bit array with 30 numbers per byte
 * (n * 30 + k with k = {7, 11, 13, 17, 19, 23, 29, 31}).
 * Multiples of prime numbers are crossed off in eratSmall_,
 * eratMedium_ and eratBig_. EratSmall is optimized for small sieving
 * primes with a lot of multiple occurrences per segment, EratMedium is
 * optimized for medium sieving primes and EratBig is optimized for
 * big sieving primes that have less than one multiple occurrence per
 * segment.
 * SieveOfEratosthenes is an abstract class, PrimeNumberGenerator and
 * PrimeNumberFinder are derived from SieveOfEratosthenes.
 */
class SieveOfEratosthenes {
public:
  enum {
    /**
     * SieveOfEratosthenes uses dense bit packing with 30 numbers
     * per byte, one byte of the sieve_ array holds the numbers 
     * n * 30 + k with k = {7, 11, 13, 17, 19, 23, 29, 31}.
     */
    NUMBERS_PER_BYTE = 30
  };
  uint64_t getStartNumber() const {
    return startNumber_;
  }
  uint64_t getStopNumber() const {
    return stopNumber_;
  }
  ResetSieve* getResetSieve() const {
    return resetSieve_;
  }
  void sieve(uint32_t);
  void finish();
protected:
  /** @see NUMBERS_PER_BYTE */
  static const uint32_t bitValues_[8];
  SieveOfEratosthenes(uint64_t, uint64_t, uint32_t, ResetSieve*);
  ~SieveOfEratosthenes();
  uint64_t getLowerBound() const {
    return lowerBound_;
  }
  virtual void analyseSieve(const uint8_t*, uint32_t) = 0;
private:
  /** The start number for this sieve of Eratosthenes object. */
  const uint64_t startNumber_;
  /** The stop number for this sieve of Eratosthenes object. */
  const uint64_t stopNumber_;
  /**
   * Value of the first byte of the sieve_ array. As
   * SieveOfEratosthenes uses 30 numbers per byte lowerBound_ is set
   * to lowerBound_ += sieveSize * 30 after each sieved segment.
   */
  uint64_t lowerBound_;
  /** Sieve of Eratosthenes array. */
  uint8_t* sieve_;
  uint32_t sieveSize_;
  /** Used to reset the sieve_ array after each sieved segment. */
  ResetSieve* const resetSieve_;
  /** Index needed by resetSieve_ to reset the sieve_ array. */
  uint32_t resetIndex_;
  /**
   * Sieve of Eratosthenes algorithm optimized for small sieving
   * primes.
   */
  EratSmall* eratSmall_;
  /**
   * Sieve of Eratosthenes algorithm optimized for medium sieving
   * primes.
   */
  EratMedium* eratMedium_;
  /**
   * Sieve of Eratosthenes algorithm optimized for big sieving
   * primes.
   */
  EratBig* eratBig_;
  uint32_t getRemainder(uint64_t);
  void initEratAlgorithms();
  void initSieve();
  void crossOffMultiples();
};

#endif /* SIEVEOFERATOSTHENES_H */
