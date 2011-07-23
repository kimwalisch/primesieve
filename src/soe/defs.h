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

/** 
 * @file defs.h 
 * @brief Macro definitions and constants that set the size of various
 *        arrays and limits within primesieve.
 *
 * The constants have been optimized for an Intel Core i5-670 3.46GHz
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
#  define NDEBUG
#endif
#include <cassert>

/**
 * @def __STDC_LIMIT_MACROS
 * Enable the UINT32_MAX, UINT64_MAX macros from <stdint.h>.
 */
#if !defined(__STDC_LIMIT_MACROS)
#  define __STDC_LIMIT_MACROS
#endif
 /**
 * @def __STDC_LIMIT_MACROS
 * Enable the UINT64_C(c) macro from <stdint.h>.
 */
#if !defined(__STDC_CONSTANT_MACROS)
#  define __STDC_CONSTANT_MACROS
#endif
#include <stdint.h>

/** Used to silence 64-bit size_t warnings. */
#define SIZEOF(x) static_cast<uint32_t> (sizeof(x))

/**
 * Reconstruct prime numbers from 1 bits of the sieve array and call a
 * callback function for each prime.
 * @remark The sieve array is of type uint8_t*
 * @see    PrimeNumberFinder.cpp, PrimeNumberGenerator.cpp
 */
#define GENERATE_PRIMES(callback, uintXX_t) {                     \
  uint32_t i = 0;                                                 \
  for (; i < sieveSize / SIZEOF(uint32_t); i++) {                 \
    uint32_t word = reinterpret_cast<const uint32_t*> (sieve)[i]; \
    while (word != 0) {                                           \
      uint32_t bitPosition = bitScanForward(word);                \
      uintXX_t prime = lowerBound + bitValues_[bitPosition];      \
      word &= word - 1;                                           \
      callback (prime);                                           \
    }                                                             \
    lowerBound += NUMBERS_PER_BYTE * SIZEOF(uint32_t);            \
  }                                                               \
  for (i *= SIZEOF(uint32_t); i < sieveSize; i++) {               \
    uint32_t byte = sieve[i];                                     \
    while (byte != 0) {                                           \
      uint32_t bitPosition = bitScanForward(byte);                \
      uintXX_t prime = lowerBound + bitValues_[bitPosition];      \
      byte &= byte - 1;                                           \
      callback (prime);                                           \
    }                                                             \
    lowerBound += NUMBERS_PER_BYTE;                               \
  }                                                               \
}

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
     * good (uses 1001 Bytes) and still fast.
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
     *    <= (sieveSize in Bytes * ERATMEDIUM_FACTOR) are added to
     * EratMedium which is used to cross off multiples.
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
  BIT0 = 0xfe, /* 11111110 */
  BIT1 = 0xfd, /* 11111101 */
  BIT2 = 0xfb, /* 11111011 */
  BIT3 = 0xf7, /* 11110111 */
  BIT4 = 0xef, /* 11101111 */
  BIT5 = 0xdf, /* 11011111 */
  BIT6 = 0xbf, /* 10111111 */
  BIT7 = 0x7f  /* 01111111 */
};

#endif /* DEFS_H */
