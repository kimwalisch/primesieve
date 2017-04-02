///
/// @file   api.cpp
/// @brief  primesieve C++ API.
///         Contains the implementations of the functions declared
///         in the primesieve.hpp header file.
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/config.hpp>
#include <primesieve/pmath.hpp>
#include <primesieve/PrimeSieve.hpp>
#include <primesieve/ParallelPrimeSieve.hpp>
#include <primesieve/Callback.hpp>
#include <primesieve.hpp>

#include <stdint.h>
#include <string>

namespace {

/// Sieve size in kilobytes used for sieving
int sieve_size = SIEVESIZE;

int num_threads = 0;

}

namespace primesieve {

//////////////////////////////////////////////////////////////////////
//                      Nth prime function
//////////////////////////////////////////////////////////////////////

uint64_t nth_prime(int64_t n, uint64_t start)
{
  ParallelPrimeSieve pps;
  pps.setSieveSize(get_sieve_size());
  pps.setNumThreads(get_num_threads());
  return pps.nthPrime(n, start);
}

//////////////////////////////////////////////////////////////////////
//                      Count functions
//////////////////////////////////////////////////////////////////////

uint64_t count_primes(uint64_t start, uint64_t stop)
{
  ParallelPrimeSieve pps;
  pps.setSieveSize(get_sieve_size());
  pps.setNumThreads(get_num_threads());
  return pps.countPrimes(start, stop);
}

uint64_t count_twins(uint64_t start, uint64_t stop)
{
  ParallelPrimeSieve pps;
  pps.setSieveSize(get_sieve_size());
  pps.setNumThreads(get_num_threads());
  return pps.countTwins(start, stop);
}

uint64_t count_triplets(uint64_t start, uint64_t stop)
{
  ParallelPrimeSieve pps;
  pps.setSieveSize(get_sieve_size());
  pps.setNumThreads(get_num_threads());
  return pps.countTriplets(start, stop);
}

uint64_t count_quadruplets(uint64_t start, uint64_t stop)
{
  ParallelPrimeSieve pps;
  pps.setSieveSize(get_sieve_size());
  pps.setNumThreads(get_num_threads());
  return pps.countQuadruplets(start, stop);
}

uint64_t count_quintuplets(uint64_t start, uint64_t stop)
{
  ParallelPrimeSieve pps;
  pps.setSieveSize(get_sieve_size());
  pps.setNumThreads(get_num_threads());
  return pps.countQuintuplets(start, stop);
}

uint64_t count_sextuplets(uint64_t start, uint64_t stop)
{
  ParallelPrimeSieve pps;
  pps.setSieveSize(get_sieve_size());
  pps.setNumThreads(get_num_threads());
  return pps.countSextuplets(start, stop);
}

//////////////////////////////////////////////////////////////////////
//                      Print functions
//////////////////////////////////////////////////////////////////////

void print_primes(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  ps.setSieveSize(get_sieve_size());
  ps.printPrimes(start, stop);
}

void print_twins(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  ps.setSieveSize(get_sieve_size());
  ps.printTwins(start, stop);
}

void print_triplets(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  ps.setSieveSize(get_sieve_size());
  ps.printTriplets(start, stop);
}

void print_quadruplets(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  ps.setSieveSize(get_sieve_size());
  ps.printQuadruplets(start, stop);
}

void print_quintuplets(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  ps.setSieveSize(get_sieve_size());
  ps.printQuintuplets(start, stop);
}

void print_sextuplets(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  ps.setSieveSize(get_sieve_size());
  ps.printSextuplets(start, stop);
}

//////////////////////////////////////////////////////////////////////
//                      Getters and Setters
//////////////////////////////////////////////////////////////////////

int get_sieve_size()
{
  return sieve_size;
}

int get_num_threads()
{
  if (num_threads)
    return num_threads;
  else
    return ParallelPrimeSieve::getMaxThreads();
}

void set_sieve_size(int kilobytes)
{
  sieve_size = inBetween(1, kilobytes, 2048);
}

void set_num_threads(int threads)
{
  num_threads = inBetween(1, threads, ParallelPrimeSieve::getMaxThreads());
}

uint64_t get_max_stop()
{
  return std::numeric_limits<uint64_t>::max();
}

//////////////////////////////////////////////////////////////////////
//                      Miscellaneous
//////////////////////////////////////////////////////////////////////

std::string primesieve_version()
{
  return PRIMESIEVE_VERSION;
}

} // namespace
