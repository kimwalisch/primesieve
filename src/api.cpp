///
/// @file   api.cpp
/// @brief  primesieve C++ API.
///         Contains the implementations of the functions declared
///         in the primesieve.hpp header file.
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
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
  // user specified sieve size
  if (sieve_size)
    return sieve_size;

  // Shared CPU caches are usually slow. Hence we only use
  // the L2 cache for sieving if each physical CPU core
  // has a private L2 cache. Also we only use half of the
  // L2 cache for the sieve array so that other important
  // data structures can also fit into the L2 cache.
  if (cpuInfo.hasPrivateL2Cache())
  {
    // convert bytes to KiB
    size_t size = cpuInfo.l2CacheSize() >> 10;
    size = size - 1;
    size = inBetween(32, size, 4096);
    size = floorPow2(size);
    return (int) size;
  }
  else if (cpuInfo.hasL1Cache())
  {
    // convert bytes to KiB
    size_t size = cpuInfo.l1CacheSize() >> 10;
    size = inBetween(8, size, 4096);
    size = floorPow2(size);
    return (int) size;
  }
  else
  {
    // default sieve size in KiB
    size_t size = 32;
    size = inBetween(8, size, 4096);
    size = floorPow2(size);
    return (int) size;
  }
}

} // namespace
