///
/// @file   config.hpp
/// @brief  Constants that set various limits within primesieve.
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef CONFIG_PRIMESIEVE_HPP
#define CONFIG_PRIMESIEVE_HPP

#include <stdint.h>

/// Disable assert() macro
#if !defined(DEBUG) && !defined(NDEBUG)
  #define NDEBUG
#endif

/// Default CPU L1 data cache size in kilobytes (per core)
#ifndef L1_DCACHE_SIZE
  #define L1_DCACHE_SIZE 32
#endif

namespace primesieve {

/// byte_t must be unsigned in primesieve
typedef unsigned char byte_t;
typedef unsigned int uint_t;

enum {
  /// SieveOfEratosthenes objects use a bit array with 30 numbers per
  /// byte for sieving, the 8 bits of each byte correspond to the
  /// offsets { 7, 11, 13, 17, 19, 23, 29, 31 }.
  ///
  NUMBERS_PER_BYTE = 30
};

namespace config {

enum {
  /// Number of sieving primes per Bucket in EratSmall, EratMedium and
  /// EratBig objects, affects performance by about 3%.
  ///
  /// - For x86-64 CPUs after  2010 use 1024
  /// - For x86-64 CPUs before 2010 use 512
  /// - For PowerPC G4 CPUs    2003 use 256
  ///
  BUCKETSIZE = 1 << 10,

  /// EratBig allocates BYTES_PER_ALLOC of new memory each time
  /// it needs more buckets. Default = 8 megabytes.
  ///
  BYTES_PER_ALLOC = (1 << 20) * 8,

  /// primesieve::iterator caches at least MIN_CACHE_ITERATOR
  /// bytes of primes. L3_CACHE_SIZE is a good value.
  ///
  MIN_CACHE_ITERATOR = (1 << 20) * 8,

  /// primesieve::iterator maximum cache size in bytes, used if
  /// pi(sqrt(n)) * 8 bytes > MAX_CACHE_ITERATOR.
  ///
  MAX_CACHE_ITERATOR = (1 << 20) * 1024
};

  /// Sieving primes <= (sieveSize in bytes * FACTOR_ERATSMALL)
  /// are processed in EratSmall objects, speed up ~ 5%.
  /// @pre FACTOR_ERATSMALL >= 0 && <= 3
  ///
  /// - For x86-64 CPUs after  2010 use 0.5
  /// - For x86-64 CPUs before 2010 use 0.8
  /// - For PowerPC G4 CPUs    2003 use 1.0
  ///
  const double FACTOR_ERATSMALL = 0.5;

  /// Sieving primes <= (sieveSize in bytes * FACTOR_ERATMEDIUM)
  /// (and > EratSmall see above) are processed in EratMedium objects.
  /// @pre FACTOR_ERATMEDIUM >= 0 && <= 9
  ///
  /// Statistically ideal factor for 4th Wheel is:
  /// FACTOR_ERATMEDIUM * 2 + FACTOR_ERATMEDIUM * 10 = 30
  /// FACTOR_ERATMEDIUM = 30 / 12
  /// FACTOR_ERATMEDIUM = 2.5
  ///
  const double FACTOR_ERATMEDIUM = 2.5;

  /// Each thread sieves at least a distance of MIN_THREAD_DISTANCE
  /// in order to reduce the initialization overhead.
  /// @pre MIN_THREAD_DISTANCE >= 100
  ///
  const uint64_t MIN_THREAD_DISTANCE = (uint64_t) 1e7;

  /// Each thread sieves at most a distance of MAX_THREAD_DISTANCE
  /// in order to prevent load imbalance near 99%.
  ///
  const uint64_t MAX_THREAD_DISTANCE = (uint64_t) 2e10;

} // namespace config
} // namespace primesieve

#endif
