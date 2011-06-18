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

#include "defs.h"

class ResetSieve;
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
  uint32_t getSieveSize() const {
    return sieveSize_;
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
  uint64_t getSegmentLow() const {
    return segmentLow_;
  }
  virtual void analyseSieve(const uint8_t*, uint32_t) = 0;
private:
  /** The start number for sieving. */
  const uint64_t startNumber_;
  /** The stop number for sieving. */
  const uint64_t stopNumber_;
  /** Lower bound of the current segment. */
  uint64_t segmentLow_;
  /** Upper bound of the current segment. */
  uint64_t segmentHigh_;
  /** Sieve of Eratosthenes array. */
  uint8_t* sieve_;
  /** Size of sieve_ in bytes. */
  uint32_t sieveSize_;
  /** Used to reset the sieve_ array after each sieved segment. */
  ResetSieve* const resetSieve_;
  /** Index needed by resetSieve_. */
  uint32_t resetIndex_;
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
  uint32_t getByteRemainder(uint64_t);
  void initEratAlgorithms();
  void initSieve();
  void crossOffMultiples();
};

#endif /* SIEVEOFERATOSTHENES_H */
