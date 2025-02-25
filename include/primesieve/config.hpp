///
/// @file   config.hpp
/// @brief  primesieve compile time constants.
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
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

/// Number of sieving primes per Bucket in EratMedium.cpp and
/// EratBig.cpp. A larger number of primes per bucket slightly
/// increases memory usage, but on the other hand decreases branch
/// mispredictions. Note that doubling the bucket size may also
/// double primesieve's memory usage for small n < 10^11 because of
/// the EratMedium.cpp algorithm, which may deteriorate
/// multi-threading performance for small n.
///
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
constexpr uint64_t MIN_CACHE_ITERATOR = 4 << 20;

/// iterator::prev_prime() maximum cache size in bytes, used
/// if pi(sqrt(n)) * 8 bytes > MAX_CACHE_ITERATOR.
///
constexpr uint64_t MAX_CACHE_ITERATOR = 1 << 30;

/// Each thread sieves at least a distance of MIN_THREAD_DISTANCE
/// in order to reduce the initialization overhead.
/// @pre MIN_THREAD_DISTANCE >= 100
///
constexpr uint64_t MIN_THREAD_DISTANCE = (uint64_t) 1e7;

/// sieveSize = sqrt(stop) * FACTOR_SIEVESIZE.
///
/// Using a larger FACTOR_SIEVESIZE increases the segment size in the
/// sieve of Eratosthenes and hence reduces the number of operations
/// used by the algorithm. However, as a drawback a larger segment
/// size is less cache efficient and hence performance may deteriorate
/// on CPUs with limited L2 cache bandwidth (especially when using
/// multi-threading).
///
/// Using FACTOR_SIEVESIZE = 2.0 performs well for counting the
/// primes < 10^11 using multi-threading on both the Apple M3 CPU and
/// the Intel Arrow Lake 245K CPU (from 2024).
///
constexpr double FACTOR_SIEVESIZE = 2.0;

/// Sieving primes <= (L1D_CACHE_BYTES * FACTOR_ERATSMALL) are
/// processed in EratSmall. When FACTOR_ERATSMALL is small fewer
/// sieving primes are processed in EratSmall.cpp and more sieving
/// primes are processed in EratMedium.cpp.
///
/// Using a larger FACTOR_ERATSMALL decreases the number of executed
/// instructions, reduces the memory usage and thereby decreases cache
/// misses but on the other hand increases branch mispredictions. In
/// my tests using a smaller FACTOR_ERATSMALL often improved single
/// thread performance, but decreased multi-threading performance. On
/// newer CPUs a smaller FACTOR_ERATSMALL is often faster.
///
/// @pre FACTOR_ERATSMALL >= 0 && <= 4.5
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
constexpr double FACTOR_ERATMEDIUM = 3.0;

} // namespace config
} // namespace

#endif
