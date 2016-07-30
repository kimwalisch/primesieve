///
/// @file   primesieve-api.cpp
/// @brief  Contains the implementations of the functions declared in
///         the primesieve.hpp header file.
///
/// Copyright (C) 2016 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/config.hpp>
#include <primesieve/pmath.hpp>
#include <primesieve/PrimeSieve.hpp>
#include <primesieve/ParallelPrimeSieve.hpp>
#include <primesieve/Callback.hpp>
#include <primesieve/PrimeFinder.hpp>
#include <primesieve.hpp>

#include <stdint.h>
#include <limits>
#include <string>

namespace {

int num_threads = primesieve::ParallelPrimeSieve::getMaxThreads();

/// Sieve size in kilobytes used for sieving
int sieve_size = SIEVESIZE;

}

namespace primesieve {

//////////////////////////////////////////////////////////////////////
//                     Nth prime functions
//////////////////////////////////////////////////////////////////////

uint64_t nth_prime(int64_t n, uint64_t start)
{
  PrimeSieve ps;
  ps.setSieveSize(get_sieve_size());
  return ps.nthPrime(n, start);
}

uint64_t parallel_nth_prime(int64_t n, uint64_t start)
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
  PrimeSieve ps;
  ps.setSieveSize(get_sieve_size());
  return ps.countPrimes(start, stop);
}

uint64_t count_twins(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  ps.setSieveSize(get_sieve_size());
  return ps.countTwins(start, stop);
}

uint64_t count_triplets(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  ps.setSieveSize(get_sieve_size());
  return ps.countTriplets(start, stop);
}

uint64_t count_quadruplets(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  ps.setSieveSize(get_sieve_size());
  return ps.countQuadruplets(start, stop);
}

uint64_t count_quintuplets(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  ps.setSieveSize(get_sieve_size());
  return ps.countQuintuplets(start, stop);
}

uint64_t count_sextuplets(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  ps.setSieveSize(get_sieve_size());
  return ps.countSextuplets(start, stop);
}

//////////////////////////////////////////////////////////////////////
//                   Parallel count functions
//////////////////////////////////////////////////////////////////////

uint64_t parallel_count_primes(uint64_t start, uint64_t stop)
{
  ParallelPrimeSieve pps;
  pps.setSieveSize(get_sieve_size());
  pps.setNumThreads(get_num_threads());
  return pps.countPrimes(start, stop);
}

uint64_t parallel_count_twins(uint64_t start, uint64_t stop)
{
  ParallelPrimeSieve pps;
  pps.setSieveSize(get_sieve_size());
  pps.setNumThreads(get_num_threads());
  return pps.countTwins(start, stop);
}

uint64_t parallel_count_triplets(uint64_t start, uint64_t stop)
{
  ParallelPrimeSieve pps;
  pps.setSieveSize(get_sieve_size());
  pps.setNumThreads(get_num_threads());
  return pps.countTriplets(start, stop);
}

uint64_t parallel_count_quadruplets(uint64_t start, uint64_t stop)
{
  ParallelPrimeSieve pps;
  pps.setSieveSize(get_sieve_size());
  pps.setNumThreads(get_num_threads());
  return pps.countQuadruplets(start, stop);
}

uint64_t parallel_count_quintuplets(uint64_t start, uint64_t stop)
{
  ParallelPrimeSieve pps;
  pps.setSieveSize(get_sieve_size());
  pps.setNumThreads(get_num_threads());
  return pps.countQuintuplets(start, stop);
}

uint64_t parallel_count_sextuplets(uint64_t start, uint64_t stop)
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
//                      Callback functions
//////////////////////////////////////////////////////////////////////

void callback_primes(uint64_t start, uint64_t stop, void (*callback)(uint64_t))
{
  PrimeSieve ps;
  ps.setSieveSize(get_sieve_size());
  ps.callbackPrimes(start, stop, callback);
}

void callback_primes(uint64_t start, uint64_t stop, Callback<uint64_t>* callback)
{
  PrimeSieve ps;
  ps.setSieveSize(get_sieve_size());
  ps.callbackPrimes(start, stop, callback);
}

//////////////////////////////////////////////////////////////////////
//                      Getters and Setters
//////////////////////////////////////////////////////////////////////

uint64_t get_max_stop()
{
  return std::numeric_limits<uint64_t>::max();
}

int get_sieve_size()
{
  return sieve_size;
}

int get_num_threads()
{
  return num_threads;
}

void set_sieve_size(int kilobytes)
{
  sieve_size = inBetween(1, kilobytes, 2048);
}

void set_num_threads(int threads)
{
  if (threads == -1)
    num_threads = ParallelPrimeSieve::getMaxThreads();
  else
    num_threads = inBetween(1, threads, ParallelPrimeSieve::getMaxThreads());
}

//////////////////////////////////////////////////////////////////////
//                      Miscellaneous
//////////////////////////////////////////////////////////////////////

std::string primesieve_version()
{
  return PRIMESIEVE_VERSION;
}

} // namespace
