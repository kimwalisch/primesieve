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
 *        arrays and limits within primesieve.
 *
 * The constants have been optimized for an
 * Intel Core i5-670 3.46GHz
 * (2 x 32 KB L1 Data Cache, 2 x 256 L2 Cache, 4 MB L3 Cache)
 * from 2010.
 */

#ifndef DEFS_H
#define DEFS_H

/**
 * @def NDEBUG
 * Disable the assert macro from <cassert> if not in debug mode.
 */
#if !defined(NDEBUG) && !defined(DEBUG) && !defined(_DEBUG)
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

/**
 * @def POPCNT64(addr, i)
 * Counts the number of one bits within the next 8 bytes of an array
 * using the SSE 4.2 POPCNT instruction.
 * @see http://en.wikipedia.org/wiki/SSE4#SSE4.2
 *
 * Successfully tested with:
 *
 *   Microsoft Visual Studio 2010 (WIN32, WIN64),
 *   GNU G++ 4.5 (x86, x64),
 *   Intel C++ Compiler 12.0 (x86, x64),
 *   Oracle Solaris Studio 12 (x64).
 */
#if (defined(_MSC_VER) || defined(__INTEL_COMPILER)) && (defined(_WIN64) || defined(_WIN32))
#  if defined(_WIN64)
#    include <nmmintrin.h> 
#    define POPCNT64(addr, i) static_cast<uint32_t> \
                              (_mm_popcnt_u64(*reinterpret_cast<const uint64_t*> (&addr[i])))
#  elif defined(_WIN32)
#    include <nmmintrin.h>
#    define POPCNT64(addr, i) (_mm_popcnt_u32(*reinterpret_cast<const uint32_t*> (&addr[i])) + \
                               _mm_popcnt_u32(*reinterpret_cast<const uint32_t*> (&addr[i+4])))
#  endif
#elif defined(__SUNPRO_CC)
#  if defined(__x86_64)
#    include <nmmintrin.h>
#    define POPCNT64(addr, i) static_cast<uint32_t> \
                              (_mm_popcnt_u64(*reinterpret_cast<const uint64_t*> (&addr[i])))
#  elif defined(__i386)
#    include <nmmintrin.h>
#    define POPCNT64(addr, i) (_mm_popcnt_u32(*reinterpret_cast<const uint32_t*> (&addr[i])) + \
                               _mm_popcnt_u32(*reinterpret_cast<const uint32_t*> (&addr[i+4])))
#  endif
#elif defined(__GNUC__)
#  if defined(__x86_64__)
#    define POPCNT64(addr, i) static_cast<uint32_t> \
                              (__builtin_popcountll(*reinterpret_cast<const uint64_t*> (&addr[i])))
#  elif defined(__i386__)
#    define POPCNT64(addr, i) (__builtin_popcount(*reinterpret_cast<const uint32_t*> (&addr[i])) + \
                               __builtin_popcount(*reinterpret_cast<const uint32_t*> (&addr[i+4])))
#  endif
#endif

namespace defs {
  /**
   * Sieving primes <= (sieveSize in Bytes * ERATSMALL_FACTOR) are
   * added to EratSmall which is used to cross off multiples.
   * Default = 1.5, future CPUs might run faster with a smaller value
   * e.g. 1.25 or 1.0.
   *
   * @pre 0 >= ERATSMALL_FACTOR < 5
   * @see SieveOfEratosthenes::sieve(uint32_t)
   */
  const double ERATSMALL_FACTOR = 1.5;

  enum {
    /**
     * Default sieve size in Kilobytes of PrimeSieve and
     * ParallelPrimeSieve objects.
     * Default = CPU L1 Data Cache size.
     *
     * @pre 1 >= PRIMESIEVE_SIEVESIZE <= 8192     
     */
    PRIMESIEVE_SIEVESIZE = 64,
    /**
     * Default pre-sieve limit of PrimeSieve and ParallelPrimeSieve
     * objects. Multiples of small primes <= PRIMESIEVE_PRESIEVE_LIMIT
     * are pre-sieved to speed up the sieve of Eratosthenes.
     * Default = 19 (uses 315.7 Kilobytes), for less memory usage 13 is
     * good (uses 1001 Bytes) and still very fast.
     *
     * @pre 11 >= PRIMESIEVE_PRESIEVE_LIMIT <= 23
     * @see PreSieve.h
     * @see PrimeSieve::setPreSieveLimit(uint32_t)
     */
    PRIMESIEVE_PRESIEVE_LIMIT = 19,
    /**
     * Minimum sieve interval per thread in ParallelPrimeSieve.
     * Due to initialization overhead a single thread is faster for
     * small sieve intervals.
     * Default = 1E8
     *
     * @pre MIN_THREAD_INTERVAL >= 100
     */
    MIN_THREAD_INTERVAL = static_cast<int> (1E8),
    /**
     * Sieve size in Kilobytes of PrimeNumberGenerator which generates
     * the primes up to n^0.5 needed for sieving.
     * Default = CPU L1 Data Cache size.
     *
     * @pre 1 >= PRIMENUMBERGENERATOR_SIEVESIZE <= 8192 &&
     *      must be power of 2
     */
    PRIMENUMBERGENERATOR_SIEVESIZE = 32,
    /**
     * Pre-sieve limit of PrimeNumberGenerator which generates the
     * primes up to n^0.5 needed for sieving.
     * Default = 13 (uses 1001 Bytes), a greater value uses more
     * memory without noticeable speed up.
     *
     * @pre 11 >= PRIMENUMBERGENERATOR_PRESIEVE_LIMIT <= 23
     * @see PrimeNumberGenerator.cpp
     */
    PRIMENUMBERGENERATOR_PRESIEVE_LIMIT = 13,
    /**
     * Sieving primes > EratSmall::getLimit() &&
     *                <= (sieveSize in Bytes * ERATMEDIUM_FACTOR)
     * are added to EratMedium which is used to cross off multiples.
     * Default = 9, future CPUs might run faster with a smaller value
     * e.g. 7 or 5.
     *
     * @see SieveOfEratosthenes::sieve(uint32_t)
     */
    ERATMEDIUM_FACTOR = 9,
    /**
     * Number of WheelPrimes (i.e. sieving primes) per Bucket in
     * EratSmall and EratMedium.
     * Default = 4096 (uses 32 Kilobytes per Bucket), CPUs with more
     * than 32 KB L1 Data Cache might run faster with a greater value.
     *
     * @see Bucket in WheelFactorization.h
     */
    ERATBASE_BUCKETSIZE = 1 << 12,
    /**
     * Number of WheelPrimes (i.e. sieving primes) per Bucket in
     * EratBig.
     * Default = 1024 (uses 8 Kilobytes per Bucket), future CPUs might
     * run faster with a greater value.
     *
     * @see Bucket in WheelFactorization.h
     */
    ERATBIG_BUCKETSIZE = 1 << 10,
    /**
     * EratBig allocates ERATBIG_MEMORY_PER_ALLOC bytes of new memory
     * each time it needs more Buckets.
     * Default = 4 Megabytes.
     *
     * @see EratBig.cpp
     */
    ERATBIG_MEMORY_PER_ALLOC = (1 << 20) * 4
  };
}

/**
 * Bit patterns used with the '&' operator to unset a specific bit of
 * a byte.
 */
enum {
  BIT0 = 0xfe, // 11111110
  BIT1 = 0xfd, // 11111101
  BIT2 = 0xfb, // 11111011
  BIT3 = 0xf7, // 11110111
  BIT4 = 0xef, // 11101111
  BIT5 = 0xdf, // 11011111
  BIT6 = 0xbf, // 10111111
  BIT7 = 0x7f  // 01111111
};

#endif /* DEFS_H */
