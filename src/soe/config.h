///
/// @file   config.h
/// @brief  Macros, typedefs and constants that set various limits
///         and array sizes within primesieve.
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef CONFIG_PRIMESIEVE_H
#define CONFIG_PRIMESIEVE_H

/// Enable the UINT64_C() macro from <stdint.h>
#if !defined(__STDC_CONSTANT_MACROS)
  #define __STDC_CONSTANT_MACROS
#endif

#include <stdint.h>

/// Disable assert() macro
#if !defined(DEBUG) && !defined(NDEBUG)
  #define NDEBUG
#endif

/// Disable the default copy constructor and assignment operator
#ifndef DISALLOW_COPY_AND_ASSIGN
#define DISALLOW_COPY_AND_ASSIGN(ClassName) \
  ClassName(const ClassName&); \
  void operator=(const ClassName&)
#endif

/// Default CPU L1 data cache size in kilobytes (per core)
#ifndef L1_DCACHE_SIZE
  #define L1_DCACHE_SIZE 32
#endif

namespace soe {

/// byte_t must be unsigned in primesieve
typedef unsigned char byte_t;
typedef unsigned int  uint_t;

enum {
  /// SieveOfEratosthenes objects use a bit array with 30 numbers per
  /// byte for sieving, the 8 bits of each byte correspond to the
  /// offsets { 7, 11, 13, 17, 19, 23, 29, 31 }.
  ///
  NUMBERS_PER_BYTE = 30
};

namespace config {
enum {
  /// Default sieve size in kilobytes. Set SIEVESIZE to your CPUs L1
  /// data cache size to get the best performance.
  /// @pre SIEVESIZE >= 1 && <= 2048
  ///
  SIEVESIZE = L1_DCACHE_SIZE,

  /// Default pre-sieve limit. Multiples of primes up to this limit
  /// are pre-sieved to speed up the sieve of Eratosthenes.
  /// @pre PRESIEVE >= 13 && <= 23.
  ///
  PRESIEVE = 19,

  /// Pre-sieving is not used unless the sieving interval is >=
  /// PRESIEVE_THRESHOLD. (Pre-sieving is expensive to initialize.)
  ///
  PRESIEVE_THRESHOLD = 100000000,

  /// Number of sieving primes per Bucket in EratSmall, EratMedium and
  /// EratBig objects, affects performance by about 3%.
  ///
  /// - For x86-64 CPUs post  2010 use 1024
  /// - For x86-64 CPUs prior 2010 use 512
  /// - For PowerPC G4 CPUs   2003 use 256
  ///
  BUCKETSIZE = 1 << 10,

  /// EratBig allocates MEMORY_PER_ALLOC bytes of new memory each
  /// time it needs more buckets. Default = 4 megabytes.
  ///
  MEMORY_PER_ALLOC = (1 << 20) * 4
};

  /// Sieving primes <= (sieveSize in bytes * FACTOR_ERATSMALL)
  /// are processed in EratSmall objects, speed up ~ 5%.
  /// @pre FACTOR_ERATSMALL >= 0 && <= 3
  ///
  /// - For x86-64 CPUs post  2010 use 0.5 (or 0.3)
  /// - For x86-64 CPUs prior 2010 use 0.8
  /// - For PowerPC G4 CPUs   2003 use 1.0
  ///
  const double FACTOR_ERATSMALL = 0.5;

  /// Sieving primes <= (sieveSize in bytes * FACTOR_ERATMEDIUM)
  /// (and > EratSmall see above) are processed in EratMedium objects.
  /// @pre FACTOR_ERATMEDIUM >= 0 && <= 9
  ///
  const double FACTOR_ERATMEDIUM = 7.5;

  /// Each thread sieves at least an interval of size
  /// MIN_THREAD_INTERVAL to reduce the initialization overhead.
  /// @pre MIN_THREAD_INTERVAL >= 100
  ///
  const uint64_t MIN_THREAD_INTERVAL = static_cast<uint64_t>(1e7);

  /// Each thread sieves at most an interval of size
  /// MAX_THREAD_INTERVAL to prevent load imbalance near 99%.
  ///
  const uint64_t MAX_THREAD_INTERVAL = static_cast<uint64_t>(2e10);

} // namespace config
} // namespace soe

#endif
