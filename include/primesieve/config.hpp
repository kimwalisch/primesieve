///
/// @file   config.hpp
/// @brief  primesieve compile time constants.
///
/// Copyright (C) 2019 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <stdint.h>

namespace {
namespace config {

enum {
  /// Number of sieving primes per Bucket in EratSmall, EratMedium
  /// and EratBig objects, affects performance by about 3%.
  /// @pre BUCKET_BYTES must be a power of 2.
  ///
  /// - For x86-64 CPUs after  2010 use 8192
  /// - For x86-64 CPUs before 2010 use 4096
  /// - For PowerPC G4 CPUs    2003 use 2048
  ///
  BUCKET_BYTES = 1 << 13,

  /// The MemoryPool allocates at most MAX_ALLOC_BYTES of new
  /// memory when it runs out of buckets.
  ///
  MAX_ALLOC_BYTES = (1 << 20) * 16,

  /// iterator::prev_prime() caches at least MIN_CACHE_ITERATOR
  /// bytes of primes. Larger is usually faster but also
  /// requires more memory.
  ///
  MIN_CACHE_ITERATOR = (1 << 20) * 8,

  /// iterator::prev_prime() maximum cache size in bytes, used
  /// if pi(sqrt(n)) * 8 bytes > MAX_CACHE_ITERATOR.
  ///
  MAX_CACHE_ITERATOR = (1 << 20) * 1024
};

  /// Sieving primes <= (sieveSize in bytes * FACTOR_ERATSMALL)
  /// are processed in EratSmall. The ideal value for
  /// FACTOR_ERATSMALL has been determined experimentally by
  /// running benchmarks near 10^10.
  /// @pre FACTOR_ERATSMALL >= 0 && <= 3
  ///
  /// - For x86-64 CPUs after  2010 use 0.4
  /// - For x86-64 CPUs before 2010 use 0.8
  /// - For PowerPC G4 CPUs    2003 use 1.0
  ///
  const double FACTOR_ERATSMALL = 0.4;

  /// Sieving primes > (sieveSize in bytes * FACTOR_ERATSMALL)
  /// and <= (sieveSize in bytes * FACTOR_ERATMEDIUM)
  /// are processed in EratMedium. The ideal value for
  /// FACTOR_ERATMEDIUM has been determined experimentally by
  /// running benchmarks near 10^14.
  ///
  /// @pre FACTOR_ERATMEDIUM >= 0 && <= 9
  /// FACTOR_ERATMEDIUM * max(sieveSize) / 30 * 6 + 6 <= max(multipleIndex)
  /// FACTOR_ERATMEDIUM * 2^22 / 30 * 6 + 6 <= 2^23 - 1
  /// FACTOR_ERATMEDIUM <= ((2^23 - 7) * 30) / (2^22 * 6)
  /// FACTOR_ERATMEDIUM <= 9.999991655
  ///
  const double FACTOR_ERATMEDIUM = 5.0;

  /// Each thread sieves at least a distance of MIN_THREAD_DISTANCE
  /// in order to reduce the initialization overhead.
  /// @pre MIN_THREAD_DISTANCE >= 100
  ///
  const uint64_t MIN_THREAD_DISTANCE = (uint64_t) 1e7;

} // namespace config
} // namespace

#endif
