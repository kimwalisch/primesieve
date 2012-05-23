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


/// @file config.h
/// @brief Macro definitions and constants that set the size of
///        various arrays and limits within primesieve.
/// The constants have been optimized for my Intel Core i5-670 3.46GHz
/// (32K L1 data cache per CPU core) and DDR3-1333. You can set
/// L1_DCACHE_SIZE and BUCKETSIZE according to your CPU specifications
/// to get the best performance.

#ifndef CONFIG_PRIMESIEVE_H
#define CONFIG_PRIMESIEVE_H

/// Enable the UINT32_MAX, UINT64_MAX macros from <stdint.h>
#if !defined(__STDC_LIMIT_MACROS)
#  define __STDC_LIMIT_MACROS
#endif

/// Enable the UINT64_C(c) macro from <stdint.h>
#if !defined(__STDC_CONSTANT_MACROS)
#  define __STDC_CONSTANT_MACROS
#endif

#include <stdint.h>

/// Disable the assert macro from <cassert> if not in debug mode
#if !defined(NDEBUG) && !defined(DEBUG) && !defined(_DEBUG)
#  define NDEBUG
#endif

/// Disable static_assert() for compilers without C++11 support
#if __cplusplus <= 199711L && \
       !defined(__GXX_EXPERIMENTAL_CXX0X__) && \
      (!defined(_MSC_VER) || _MSC_VER < 1600)
#  define static_assert(expression, message) static_cast<void>(0)
#endif

/// Default CPU L1 data cache size per core in kilobytes
#if !defined(L1_DCACHE_SIZE)
  #define L1_DCACHE_SIZE 32
#endif

namespace soe {
namespace config {

/// Sieving primes <= (sieveSize in bytes * FACTOR_ERATSMALL)
/// are used with EratSmall objects.
/// @pre FACTOR_ERATSMALL >= 0 && < 5
/// @see SieveOfEratosthenes::sieve(uint32_t)
///
const double FACTOR_ERATSMALL = 0.75;

enum {
  /// Sieving primes > (sieveSize in bytes * FACTOR_ERATSMALL) &&
  ///               <= (sieveSize in bytes * FACTOR_ERATMEDIUM)
  /// are used with EratMedium objects.
  /// @pre FACTOR_ERATMEDIUM >= 0 && <= 6
  /// @see SieveOfEratosthenes::sieve(uint32_t)
  ///
  FACTOR_ERATMEDIUM = 6,

  /// Default pre-sieve limit of PrimeSieve and ParallelPrimeSieve
  /// objects. Multiples of small primes up to this limit are
  /// pre-sieved to speed up the sieve of Eratosthenes.
  /// Default = 19 (uses 315.7 kilobytes), for less memory usage 13 is
  /// good (uses 1001 bytes) and still fast.
  /// @pre PRESIEVE_LIMIT >= 13 && <= 23
  /// @see PreSieve.h
  ///
  PRESIEVE_LIMIT = 19,

  /// Default sieve size in kilobytes of PrimeSieve and
  /// ParallelPrimeSieve objects.
  /// @pre SIEVESIZE >= 1 && <= 4096
  ///
  SIEVESIZE = L1_DCACHE_SIZE,

  /// Pre-sieve limit of PrimeNumberGenerator.
  /// Default = 13 (uses 1001 bytes) a greater value uses more memory
  /// without noticeable speed speed up.
  /// @pre PRESIEVE_LIMIT_PRIMENUMBERGENERATOR >= 13 && <= 23
  ///
  PRESIEVE_LIMIT_PRIMENUMBERGENERATOR = 13,

  /// Sieve size in kilobytes of PrimeNumberGenerator which generates
  /// the primes up to sqrt(n) needed for sieving.
  /// @pre SIEVESIZE_PRIMENUMBERGENERATOR >= 1 && <= 4096
  ///
  SIEVESIZE_PRIMENUMBERGENERATOR = L1_DCACHE_SIZE,

  /// Number of WheelPrimes (i.e. sieving primes) per Bucket in
  /// EratSmall, EratMedium and EratBig objects. For Intel Core 2 CPUs
  /// from 2008 and first generation Intel Core-i CPUs from 2010 use
  /// 1024, future CPU generations are likely to perform better with a
  /// greater value.
  /// @see Bucket in WheelFactorization.h
  ///
  BUCKETSIZE = 1 << 10,

  /// EratBig allocates MEMORY_PER_ALLOC bytes of new memory each time
  /// it needs more Buckets. Default = 4 megabytes.
  ///
  MEMORY_PER_ALLOC = (1 << 20) * 4
};


/// Worker threads sieve at least an interval of size
/// MIN_THREAD_INTERVAL to reduce the thread creation overhead.
/// @pre MIN_THREAD_INTERVAL >= 100
///
const uint64_t MIN_THREAD_INTERVAL = static_cast<uint64_t>(1E8);

/// Worker threads sieve at most an interval of size
/// MAX_THREAD_INTERVAL to prevent load imbalance when lots of threads
/// are used. Default = 2E10, this setting guarantees that worker
/// threads always finish in less than a minute.
///
const uint64_t MAX_THREAD_INTERVAL = static_cast<uint64_t>(2E10);

} // namespace config
} // namespace soe

#endif
