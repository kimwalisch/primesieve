/*
 * PrimeNumberFinder.h -- This file is part of primesieve
 *
 * Copyright (C) 2010 Kim Walisch, <kim.walisch@gmail.com>
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

#ifndef PRIMENUMBERFINDER_H
#define PRIMENUMBERFINDER_H

#include "SieveOfEratosthenes.h"

#include <stdint.h>

/** Flags (settings) for PrimeNumberFinder. */
enum {
  COUNT_PRIMES      = (1 <<  0),
  COUNT_TWINS       = (1 <<  1),
  COUNT_TRIPLETS    = (1 <<  2),
  COUNT_QUADRUPLETS = (1 <<  3),
  COUNT_QUINTUPLETS = (1 <<  4),
  COUNT_SEXTUPLETS  = (1 <<  5),
  COUNT_SEPTUPLETS  = (1 <<  6),
  PRINT_PRIMES      = (1 <<  7),
  PRINT_TWINS       = (1 <<  8),
  PRINT_TRIPLETS    = (1 <<  9),
  PRINT_QUADRUPLETS = (1 << 10),
  PRINT_QUINTUPLETS = (1 << 11),
  PRINT_SEXTUPLETS  = (1 << 12),
  PRINT_SEPTUPLETS  = (1 << 13),
  PRINT_STATUS      = (1 << 14),
  STORE_STATUS      = (1 << 15),
  COUNT_FLAGS       = COUNT_PRIMES | COUNT_TWINS | COUNT_TRIPLETS | COUNT_QUADRUPLETS | COUNT_QUINTUPLETS | COUNT_SEXTUPLETS | COUNT_SEPTUPLETS,
  PRINT_FLAGS       = PRINT_PRIMES | PRINT_TWINS | PRINT_TRIPLETS | PRINT_QUADRUPLETS | PRINT_QUINTUPLETS | PRINT_SEXTUPLETS | PRINT_SEPTUPLETS,
  RESULTS_FLAGS     = STORE_STATUS | COUNT_FLAGS,
  STATUS_FLAGS      = PRINT_STATUS | STORE_STATUS 
};

/**
 * Sieve of Eratosthenes that is used to find the prime numbers and
 * prime k-tuplets (twin primes, prime triplets, ...) between a
 * startNumber_ and a stopNumber_.
 */
class PrimeNumberFinder: public SieveOfEratosthenes {
public:
  /** PrimeNumberFinder stores its results in here. */
  struct Results {
    Results() {
      this->reset(0);
    }
    enum {
      COUNTS_SIZE = 7
    };
    // prime count variables
    int64_t counts[COUNTS_SIZE];
    // status of the sieving process in percents
    volatile float status;
    void reset(uint32_t countFlags) {
      for (uint32_t i = 0; i < COUNTS_SIZE; i++)
        counts[i] = (countFlags & (COUNT_PRIMES << i)) ? 0 : -1;
      status = 0.0f;
    }
  };
  PrimeNumberFinder(uint64_t, uint64_t, uint32_t, ResetSieve*, Results*,
      uint32_t);
  ~PrimeNumberFinder();
private:
  static const uint32_t nextBitValue_[NUMBERS_PER_BYTE];
  /** Settings for PrimeNumberFinder. */
  const uint32_t flags_;
  /**
   * Used to check if the CPU supports the SSE 4.2 POPCNT
   * instruction.
   */
  const bool isPOPCNTSupported_;
  /**
   * Gives the count of prime numbers and k-tuplets within a byte
   * of the sieve_ array.
   */
  uint32_t** primeByteCounts_;
  /**
   * Lookup table used to calculate the prime number or k-tuplet
   * corresponding to a given 1 bit of the sieve_ array.
   */
  uint32_t** primeBitValues_;
  /**
   * The prime counts and the status of the sieving process are
   * saved in here.
   */
  Results* const results_;
  /** Status of the sieving process in percents. */
  uint32_t status_;
  void initLookupTables();
  void count(const uint8_t*, uint32_t);
  void print(const uint8_t*, uint32_t);
  void status(uint32_t);
  void analyseSieve(const uint8_t*, uint32_t);
};

#endif /* PRIMENUMBERFINDER_H */
