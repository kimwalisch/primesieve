/*
 * defs.h -- This file is part of primesieve
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

/** 
 * @file defs.h 
 * @brief Macro definitions and constants that set the size of various
 *        arrays and upper bounds within primesieve.
 * The values are optimized for CPUs with 32 to 64 KiloBytes of L1
 * Data Cache.
 */

#ifndef DEFS_H
#define DEFS_H

/**
 * @def NDEBUG
 * Disable the assert macro from <cassert> if not in debug mode.
 */
#if !defined(DEBUG) && !defined(_DEBUG)
#  define NDEBUG 1
#endif

/**
 * @def __STDC_LIMIT_MACROS
 * Enable the UINT64_MAX, UINT32_MAX macros from <stdint.h>.
 */
 /**
 * @def __STDC_LIMIT_MACROS
 * Enable the UINT64_C(c) macro from <stdint.h>.
 */
#if !defined(__STDC_LIMIT_MACROS)
#  define __STDC_LIMIT_MACROS 1
#endif
#if !defined(__STDC_CONSTANT_MACROS)
#  define __STDC_CONSTANT_MACROS  1
#endif
#include <stdint.h>

namespace defs {
  /**
   * (sieveSize * FACTOR_ERATSMALL) is the upper bound for sieving
   * primes used with EratSmall.
   * Default = 1.5, a value between 0.75 and 2.5 is reasonable
   * @pre < 5 && <= FACTOR_ERATMEDIUM
   * @see SieveOfEratosthenes.cpp
   */
  const double FACTOR_ERATSMALL = 1.5;

  enum {
    /**
     * Multiples of prime numbers up to this number will be eliminated
     * without sieving if the sieve interval is not too small >= 1E9
     * else 13 is used.
     * Default = 19 (uses 315.7 KiloBytes) for less memory usage 13 is
     * good (uses 1001 Bytes) and still very fast
     * @pre a prime number >= 13 && <= 23
     * @see ResetSieve.h & .cpp
     */
    LIMIT_RESETSIEVE = 19,
    /**
     * Sieve size of the secondary sieve of Eratosthenes
     * (PrimeNumberGenerator) that generates the prime numbers up to
     * sqrt(stopNumber) needed for sieving.
     * Default = CPU L1 cache size
     * @pre >= 1024 && 
     *      <= 2^23 && 
     *      must be a power of 2
     * @see PrimeNumberGenerator.cpp
     */
    SIEVESIZE_PRIMENUMBERGENERATOR = 1024 * 32,
    /**
     * Default sieve size of the main sieve of Eratosthenes
     * (PrimeNumberFinder), is used if the user does not set his own
     * sieve size.
     * Default = CPU L1 cache size
     * @pre >= 1024 && 
     *      <= 2^23 && 
     *      must be a power of 2
     * @see PrimeSieve.cpp
     */
    SIEVESIZE_PRIMENUMBERFINDER = 1024 * 64,
    /**
     * (sieveSize * FACTOR_ERATMEDIUM) is the upper bound for sieving
     * primes used with EratMedium.
     * Default = 9, a value between 5 and 15 is reasonable
     * @pre >= FACTOR_ERATSMALL
     * @see SieveOfEratosthenes.cpp
     */
    FACTOR_ERATMEDIUM = 9,
    /**
     * Size of a bucket array within EratBase, buckets are used to
     * store sieving primes.
     * Default = 4096 (32 KiloBytes)
     */
    BUCKETSIZE_ERATBASE = 1 << 12,
    /**
     * Size of a bucket array within EratBig, buckets are used to
     * store sieving primes.
     * Default = 1024 (8 KiloBytes)
     */
    BUCKETSIZE_ERATBIG = 1 << 10,
    /**
     * Allocate MEMORY_PER_ALLOC_ERATBIG bytes of new memory each time
     * EratBig needs more buckets.
     * Default = 4 MegaBytes
     */
    MEMORY_PER_ALLOC_ERATBIG = (1 << 20) * 4
  };
}

/**
 * Bit patterns used with the '&' operator to unset a specific bit of
 * a byte.
 */
enum {
  BIT0 = 0xfe, // 1111.1110
  BIT1 = 0xfd, // 1111.1101
  BIT2 = 0xfb, // 1111.1011
  BIT3 = 0xf7, // 1111.0111
  BIT4 = 0xef, // 1110.1111
  BIT5 = 0xdf, // 1101.1111
  BIT6 = 0xbf, // 1011.1111
  BIT7 = 0x7f  // 0111.1111
};

#endif /* DEFS_H */
