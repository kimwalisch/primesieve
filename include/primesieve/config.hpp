///
/// @file   config.hpp
/// @brief  primesieve compile time constants.
///
/// Copyright (C) 2021 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <stdint.h>

namespace {
namespace config {

/// Fallback L1 data cache size per core (in bytes) that will
/// be used if the CpuInfo class is unable to detect the CPU's
/// L1 data cache size at runtime.
///
constexpr uint64_t L1D_CACHE_BYTES = 32 << 10;

/// Fallback sieve array size (in bytes) that will be used if
/// the CpuInfo class is unable to detect the CPU's cache sizes
/// at runtime and the user has not set the sieve size.
/// @see get_sieve_size() in api.cpp.
///
/// The best performance is usually achieved using a sieve
/// size that matches the CPU's L1 data cache size per core or
/// that is slightly larger than the CPU's L1 data cache size
/// but smaller than the L2 cache size per core.
///
constexpr uint64_t SIEVE_BYTES = L1D_CACHE_BYTES * 8;

/// Number of sieving primes per Bucket in EratSmall, EratMedium
/// and EratBig objects, affects performance by about 3%.
/// @pre BUCKET_BYTES must be a power of 2.
///
/// - For x86-64 CPUs after  2010 use 8192
/// - For x86-64 CPUs before 2010 use 4096
/// - For PowerPC G4 CPUs    2003 use 2048
///
constexpr uint64_t BUCKET_BYTES = 8 << 10;

/// The MemoryPool allocates at most MAX_ALLOC_BYTES of new
/// memory when it runs out of buckets.
///
constexpr uint64_t MAX_ALLOC_BYTES = 16 << 20;

/// iterator::prev_prime() caches at least MIN_CACHE_ITERATOR
/// bytes of primes. Larger is usually faster but also
/// requires more memory.
///
constexpr uint64_t MIN_CACHE_ITERATOR = 8 << 20;

/// iterator::prev_prime() maximum cache size in bytes, used
/// if pi(sqrt(n)) * 8 bytes > MAX_CACHE_ITERATOR.
///
constexpr uint64_t MAX_CACHE_ITERATOR = 1 << 30;

/// Each thread sieves at least a distance of MIN_THREAD_DISTANCE
/// in order to reduce the initialization overhead.
/// @pre MIN_THREAD_DISTANCE >= 100
///
constexpr uint64_t MIN_THREAD_DISTANCE = (uint64_t) 1e7;

/// Sieving primes <= (L1D_CACHE_BYTES * FACTOR_ERATSMALL)
/// are processed in EratSmall. The ideal value for
/// FACTOR_ERATSMALL has been determined experimentally by
/// running benchmarks near 10^10.
/// @pre FACTOR_ERATSMALL >= 0 && <= 3
///
constexpr double FACTOR_ERATSMALL = 0.2;

/// Sieving primes > (sieveSize in bytes * FACTOR_ERATSMALL)
/// and <= (sieveSize in bytes * FACTOR_ERATMEDIUM)
/// are processed in EratMedium.
///
/// When FACTOR_ERATMEDIUM is small fewer sieving primes are
/// processed in EratMedium.cpp and more sieving primes are
/// processed in EratBig.cpp. Generally a larger FACTOR_ERATMEDIUM
/// decreases the number of executed instructions, but increases
/// the number of branch mispredictions. On newer CPUs it is more
/// important to reduce the number of branch mispredictions than to
/// aim for the fewest number of executed instructions.
///
/// @pre FACTOR_ERATMEDIUM >= 0 && <= 4.5
/// FACTOR_ERATMEDIUM * max(sieveSize) / 30 * 6 + 6 <= max(multipleIndex)
/// FACTOR_ERATMEDIUM * 2^23 / 30 * 6 + 6 <= 2^23 - 1
/// FACTOR_ERATMEDIUM <= ((2^23 - 7) * 30) / (2^23 * 6)
/// FACTOR_ERATMEDIUM <= 4.99999582
///
constexpr double FACTOR_ERATMEDIUM = 1.75;

} // namespace config
} // namespace

#endif
