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

/** 
 * @file defs.h 
 * @brief Macro definitions and constants that set the size of various
 *        arrays and limits within primesieve.
 *
 * The constants have been optimized for my Intel Core i5-670 3.46GHz
 * (2x 32K L1 Data Cache, 2x 256K L2 Cache) and DDR3-1333.
 * You can set L1_DCACHE_SIZE, L2_CACHE_SIZE and BUCKET_SIZE according
 * to your CPU specifications to get the best performance.
 */

#ifndef DEFS_H
#define DEFS_H

/**
 * @def L1_DCACHE_SIZE
 * The CPU's L1 data cache size per core in kilobytes.
 */
#if !defined(L1_DCACHE_SIZE)
  #define L1_DCACHE_SIZE 32
#endif
/**
 * @def L2_CACHE_SIZE
 * The CPU's L2 cache size per core in kilobytes.
 */
#if !defined(L2_CACHE_SIZE)
  #define L2_CACHE_SIZE 256
#endif

/**
 * @def NDEBUG
 * Disable the assert macro from <cassert> if not in debug mode.
 */
#if !defined(NDEBUG) && !defined(DEBUG) && !defined(_DEBUG)
#  define NDEBUG
#endif
#include <cassert>

/**
 * @def static_assert(expression, message)
 * Disable static_assert() for compilers without C++11 support.
 */
#if __cplusplus <= 199711L && !defined(__GXX_EXPERIMENTAL_CXX0X__)
#  define static_assert(expression, message) static_cast<void> (0)
#endif

/**
 * @def __STDC_LIMIT_MACROS
 * Enable the UINT32_MAX, UINT64_MAX macros from <stdint.h>.
 */
#if !defined(__STDC_LIMIT_MACROS)
#  define __STDC_LIMIT_MACROS
#endif
 /**
 * @def __STDC_CONSTANT_MACROS
 * Enable the UINT64_C(c) macro from <stdint.h>.
 */
#if !defined(__STDC_CONSTANT_MACROS)
#  define __STDC_CONSTANT_MACROS
#endif
#include <stdint.h>

/** Used to silence 64-bit size_t warnings. */
#define SIZEOF(x) static_cast<uint32_t> (sizeof(x))

/**
 * Reconstruct prime numbers from 1 bits of the sieve array
 * and call a callback function for each prime.
 * @see PrimeNumberFinder.cpp, PrimeNumberGenerator.cpp
 */
#define GENERATE_PRIMES(callback, uintXX_t) {                      \
  uintXX_t lowerBound = static_cast<uintXX_t> (getSegmentLow());   \
  uint32_t i = 0;                                                  \
  for (; i < sieveSize / SIZEOF(uint32_t); i++) {                  \
    uint32_t dword = reinterpret_cast<const uint32_t*> (sieve)[i]; \
    while (dword != 0) {                                           \
      uint32_t bitPosition = bitScanForward(dword);                \
      uintXX_t prime = lowerBound + bitValues_[bitPosition];       \
      dword &= dword - 1;                                          \
      callback (prime);                                            \
    }                                                              \
    lowerBound += NUMBERS_PER_BYTE * SIZEOF(uint32_t);             \
  }                                                                \
  for (i *= SIZEOF(uint32_t); i < sieveSize; i++) {                \
    uint32_t byte = sieve[i];                                      \
    while (byte != 0) {                                            \
      uint32_t bitPosition = bitScanForward(byte);                 \
      uintXX_t prime = lowerBound + bitValues_[bitPosition];       \
      byte &= byte - 1;                                            \
      callback (prime);                                            \
    }                                                              \
    lowerBound += NUMBERS_PER_BYTE;                                \
  }                                                                \
}

namespace defs {
  /**
   * Sieving primes <= (sieveSize in bytes * ERATSMALL_FACTOR) are
   * used with EratSmall objects.
   * @pre ERATSMALL_FACTOR >= 0 && < 5
   * @see SieveOfEratosthenes::sieve(uint32_t)
   */
  const double ERATSMALL_FACTOR = 0.75;

  enum {
    /**
     * Sieving primes >  (sieveSize in bytes * ERATSMALL_FACTOR) &&
     *                <= (sieveSize in bytes * ERATMEDIUM_FACTOR)
     * are used with EratMedium objects.
     * @pre ERATMEDIUM_FACTOR >= 0 && <= 6
     * @see SieveOfEratosthenes::sieve(uint32_t)
     */
    ERATMEDIUM_FACTOR = 6,
    /**
     * Default pre-sieve limit of PrimeSieve and ParallelPrimeSieve
     * objects, multiples of small primes up to this limit are
     * pre-sieved to speed up the sieve of Eratosthenes.
     * Default = 19 (uses 315.7 kilobytes), for less memory usage 13 is
     * good (uses 1001 bytes) and still fast.
     * @pre PRIMESIEVE_PRESIEVE_LIMIT >= 13 && <= 23
     * @see PreSieve.h
     */
    PRIMESIEVE_PRESIEVE_LIMIT = 19,
    /**
     * Default sieve size in kilobytes of PrimeSieve and
     * ParallelPrimeSieve objects.
     * @pre PRIMESIEVE_SIEVESIZE >= 1 && <= 4096
     */
    PRIMESIEVE_SIEVESIZE = L1_DCACHE_SIZE,
    /**
     * Pre-sieve limit of PrimeNumberGenerator, default = 13 (uses
     * 1001 bytes) a greater value uses more memory without noticeable
     * speed up.
     * @pre PRIMENUMBERGENERATOR_PRESIEVE_LIMIT >= 13 && <= 23
     */
    PRIMENUMBERGENERATOR_PRESIEVE_LIMIT = 13,
    /**
     * Sieve size in kilobytes of PrimeNumberGenerator which generates
     * the primes up to sqrt(n) needed for sieving.
     * @pre PRIMENUMBERGENERATOR_SIEVESIZE >= 1 && <= 4096
     */
    PRIMENUMBERGENERATOR_SIEVESIZE = L1_DCACHE_SIZE,
    /**
     * Number of WheelPrimes (i.e. sieving primes) per Bucket in
     * Erat(Small|Medium|Big) objects.
     * For Intel Core 2 CPUs from 2008 and first generation Intel
     * Core-i CPUs from 2010 use 1024, future CPU generations are
     * likely to perform better with a greater value.
     * @see Bucket in WheelFactorization.h
     */
    BUCKET_SIZE = 1 << 10,
    /**
     * EratBig allocates ERATBIG_MEMORY_PER_ALLOC bytes of new memory
     * each time it needs more Buckets.
     * Default = 4 megabytes.
     */
    ERATBIG_MEMORY_PER_ALLOC = (1 << 20) * 4,
    /**
     * For performance reasons each thread sieves at least an interval
     * of size MIN_THREAD_INTERVAL in ParallelPrimeSieve::sieve().
     * @pre MIN_THREAD_INTERVAL >= 100
     */
    MIN_THREAD_INTERVAL = static_cast<int> (1E8)
  };
}

/**
 * Bit patterns used with the '&' operator to unset
 * a specific bit of a byte.
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
