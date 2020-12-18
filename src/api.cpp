///
/// @file   api.cpp
/// @brief  primesieve C++ API.
///         Contains the implementations of the functions declared
///         in the primesieve.hpp header file.
///
/// Copyright (C) 2020 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve.hpp>
#include <primesieve/CpuInfo.hpp>
#include <primesieve/pmath.hpp>
#include <primesieve/PrimeSieve.hpp>
#include <primesieve/ParallelSieve.hpp>

#include <stdint.h>
#include <cstddef>
#include <limits>
#include <string>

namespace {

int sieve_size = 0;

int num_threads = 0;

}

namespace primesieve {

uint64_t nth_prime(int64_t n, uint64_t start)
{
  ParallelSieve ps;
  return ps.nthPrime(n, start);
}

uint64_t count_primes(uint64_t start, uint64_t stop)
{
  ParallelSieve ps;
  ps.sieve(start, stop, COUNT_PRIMES);
  return ps.getCount(0);
}

uint64_t count_twins(uint64_t start, uint64_t stop)
{
  ParallelSieve ps;
  ps.sieve(start, stop, COUNT_TWINS);
  return ps.getCount(1);
}

uint64_t count_triplets(uint64_t start, uint64_t stop)
{
  ParallelSieve ps;
  ps.sieve(start, stop, COUNT_TRIPLETS);
  return ps.getCount(2);
}

uint64_t count_quadruplets(uint64_t start, uint64_t stop)
{
  ParallelSieve ps;
  ps.sieve(start, stop, COUNT_QUADRUPLETS);
  return ps.getCount(3);
}

uint64_t count_quintuplets(uint64_t start, uint64_t stop)
{
  ParallelSieve ps;
  ps.sieve(start, stop, COUNT_QUINTUPLETS);
  return ps.getCount(4);
}

uint64_t count_sextuplets(uint64_t start, uint64_t stop)
{
  ParallelSieve ps;
  ps.sieve(start, stop, COUNT_SEXTUPLETS);
  return ps.getCount(5);
}

void print_primes(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  ps.sieve(start, stop, PRINT_PRIMES);
}

void print_twins(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  ps.sieve(start, stop, PRINT_TWINS);
}

void print_triplets(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  ps.sieve(start, stop, PRINT_TRIPLETS);
}

void print_quadruplets(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  ps.sieve(start, stop, PRINT_QUADRUPLETS);
}

void print_quintuplets(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  ps.sieve(start, stop, PRINT_QUINTUPLETS);
}

void print_sextuplets(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  ps.sieve(start, stop, PRINT_SEXTUPLETS);
}

int get_num_threads()
{
  if (num_threads)
    return num_threads;
  else
    return ParallelSieve::getMaxThreads();
}

void set_num_threads(int threads)
{
  num_threads = inBetween(1, threads, ParallelSieve::getMaxThreads());
}

uint64_t get_max_stop()
{
  return std::numeric_limits<uint64_t>::max();
}

std::string primesieve_version()
{
  return PRIMESIEVE_VERSION;
}

void set_sieve_size(int size)
{
  sieve_size = inBetween(8, size, 4096);
  sieve_size = floorPow2(sieve_size);
}

int get_sieve_size()
{
  // User specified sieve size
  if (sieve_size)
    return sieve_size;

#if defined(__APPLE__)
  // On Apple's operating systems (macOS & iOS) we use
  // the sysctl library to query CPU info. Unfortunately
  // sysctl returns erroneous L2 & L3 cache information on
  // Apple silicon CPUs (ARM & ARM64). We can only know
  // if an L2 cache exists and that it is likely fast. If
  // this is the case, we make a conservative guess that
  // the L2 cache per core is at least 8x larger than the
  // L1 cache as this will likely improve performance.
  if (cpuInfo.sysctlL2CacheWorkaround() &&
      cpuInfo.hasL1Cache() &&
      cpuInfo.l2CacheSize() > cpuInfo.l1CacheSize() * 8)
  {
    // Convert bytes to KiB
    size_t size = cpuInfo.l1CacheSize() >> 10;
    size = inBetween(8, size * 8, 4096);
    size = floorPow2(size);
    return (int) size;
  }
#endif

  // Shared CPU caches are usually slow. Hence we only use
  // the L2 cache for sieving if each physical CPU core
  // has a private L2 cache. Also we only use half of the
  // L2 cache. This is a safety measure as some CPUs incur
  // a significant performance degradation if we fully
  // utilize the L2 cache.
  if (cpuInfo.hasPrivateL2Cache())
  {
    // Convert bytes to KiB
    size_t size = cpuInfo.l2CacheSize() >> 10;
    size = inBetween(32, size - 1, 4096);
    size = floorPow2(size);
    return (int) size;
  }

  // TODO: Shared L2 caches can also be fast?!
  //
  // Up until 2020 shared L2 caches have been slow when
  // used with multi-threading in primesieve on all
  // desktop and server CPUs. However in 2020 Apple
  // released their M1 CPU (ARM64) with an L2 cache that
  // performs well using multi-threading in primesieve and
  // media seems to agree that this L2 cache is shared.
  // I still have some doubts whether this is really the
  // case, but if it's really true then it is likely that
  // competitors will eventually catch up and also build
  // CPUs with fast shared L2 caches.
  //
  // If the CPU manufacturers move to fast L2 shared
  // caches then we have to add support for it here. We
  // should then set the sieve size to e.g.:
  // total L2 cache size / (L2 cache sharing * 2).

  if (cpuInfo.hasL1Cache())
  {
    // Convert bytes to KiB
    size_t size = cpuInfo.l1CacheSize() >> 10;
    size = inBetween(8, size, 4096);
    size = floorPow2(size);
    return (int) size;
  }
  else
  {
    // Default sieve size in KiB
    size_t size = 32;
    size = inBetween(8, size, 4096);
    size = floorPow2(size);
    return (int) size;
  }
}

} // namespace
