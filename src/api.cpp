///
/// @file   api.cpp
/// @brief  primesieve C++ API.
///         Contains the implementations of the functions declared
///         in the primesieve.hpp header file.
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include "CpuInfo.hpp"
#include "PrimeSieveClass.hpp"
#include "ParallelSieve.hpp"

#include <primesieve.hpp>
#include <primesieve/config.hpp>
#include <primesieve/pmath.hpp>

#include <stdint.h>
#include <cstddef>
#include <limits>
#include <string>

using std::size_t;

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
  sieve_size = inBetween(16, size, 8192);
}

int get_sieve_size()
{
  // User specified sieve size
  if (sieve_size)
    return sieve_size;

  if (cpuInfo.hasL1Cache() &&
      cpuInfo.hasL2Cache())
  {
    // Convert bytes to KiB
    size_t l1Size = cpuInfo.l1CacheBytes() >> 10;
    size_t l2Size = cpuInfo.l2CacheBytes() >> 10;

    // Check if the CPU cache info is likely correct.
    // When primesieve is run inside a virtual machine
    // the cache sharing info is often reported as 1
    // which is often incorrect. Hence if at least one
    // of the CPU caches sharing info is > 1, then we
    // assume that the reported values are correct.
    if (cpuInfo.hasL2Sharing() && (cpuInfo.l2Sharing() > 1 ||
        (cpuInfo.hasL3Sharing() && cpuInfo.l3Sharing() > 1)))
    {
      size_t maxSize = l2Size / cpuInfo.l2Sharing();

      // Many CPUs have scaling issues when running
      // multi-threaded workloads and fully utilizing
      // the L2 cache. Hence we ensure that the sieve
      // array size is < L2 cache size (per core).
      if (cpuInfo.l2Sharing() == 2)
        maxSize = floorPow2(maxSize);
      else
        maxSize = floorPow2(maxSize - 1);

      maxSize = std::max(l1Size, maxSize);
      size_t size = std::min(l1Size * 16, maxSize);
      size = inBetween(16, size, 8192);
      return (int) size;
    }
    else
    {
      // In this code path we cannot trust the CPU cache
      // info reported by the OS. Hence, we are more
      // conservative and use a smaller sieve array size.
      size_t maxSize = floorPow2(l2Size - 1);
      maxSize = std::max(l1Size, maxSize);
      size_t size = std::min(l1Size * 8, maxSize);
      size = inBetween(16, size, 8192);
      return (int) size;
    }
  }
  else if (cpuInfo.hasL1Cache())
  {
    // Convert bytes to KiB
    size_t l1Size = cpuInfo.l1CacheBytes() >> 10;
    l1Size = inBetween(16, l1Size, 8192);
    return (int) l1Size;
  }
  else
  {
    // Default sieve size in KiB
    size_t l1Size = config::L1D_CACHE_BYTES >> 10;
    size_t size = l1Size * 8;
    size = inBetween(16, size, 8192);
    return (int) size;
  }
}

} // namespace
