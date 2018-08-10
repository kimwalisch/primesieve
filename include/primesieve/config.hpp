///
/// @file   config.hpp
/// @brief  primesieve compile time constants.
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <stdint.h>

/// Disable assert() by default
#if !defined(DEBUG) && !defined(NDEBUG)
  #define NDEBUG
#endif

namespace primesieve {
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
  /// it needs more buckets.
  ///
  BYTES_PER_ALLOC = (1 << 20) * 8,

  /// primesieve::iterator caches at least MIN_CACHE_ITERATOR
  /// bytes of primes. Larger is usually faster but also
  /// requires more memory.
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
  /// @pre FACTOR_ERATMEDIUM >= 0 && <= 5
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

} // namespace config
} // namespace primesieve

#endif
