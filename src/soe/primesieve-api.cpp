///
/// @file   primesieve-api.cpp
/// @brief  This file contains convenience functions for the
///         most useful methods of the PrimeSieve and
///         ParallelPrimeSieve classes.
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve.h>
#include <primesieve/soe/PrimeFinder.h>

#include <stdint.h>
#include <string>

namespace primesieve
{

void callback_primes(uint64_t start, uint64_t stop, void (*callback)(uint64_t))
{
  PrimeSieve ps;
  ps.callbackPrimes(start, stop, callback);
}

void callback_primes(uint64_t start, uint64_t stop, PrimeSieveCallback<uint64_t>* callback)
{
  PrimeSieve ps;
  ps.callbackPrimes(start, stop, callback);
}

uint64_t count_primes(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  return ps.countPrimes(start, stop);
}

uint64_t count_twins(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  return ps.countTwins(start, stop);
}

uint64_t count_triplets(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  return ps.countTriplets(start, stop);
}

uint64_t count_quadruplets(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  return ps.countQuadruplets(start, stop);
}

uint64_t count_quintuplets(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  return ps.countQuintuplets(start, stop);
}

uint64_t count_sextuplets(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  return ps.countSextuplets(start, stop);
}

uint64_t count_septuplets(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  return ps.countSeptuplets(start, stop);
}

uint64_t max_stop()
{
  return soe::PrimeFinder::getMaxStop();
}

std::string max_stop_string()
{
  return soe::PrimeFinder::getMaxStopString();
}

uint64_t nth_prime(uint64_t n, uint64_t start)
{
  PrimeSieve ps;
  return ps.nthPrime(n, start);
}

uint64_t nth_prime(uint64_t n)
{
  return nth_prime(n, 0);
}

void parallel_callback_primes(uint64_t start, uint64_t stop, void (*callback)(uint64_t), int threads)
{
  ParallelPrimeSieve pps;
  pps.setNumThreads(threads);
  pps.callbackPrimes(start, stop, callback);
}

void parallel_callback_primes(uint64_t start, uint64_t stop, void (*callback)(uint64_t, int), int threads)
{
  ParallelPrimeSieve pps;
  pps.setNumThreads(threads);
  pps.callbackPrimes(start, stop, callback);
}

void parallel_callback_primes(uint64_t start, uint64_t stop, void (*callback)(uint64_t, int))
{
  parallel_callback_primes(start, stop, callback, MAX_THREADS);
}

void parallel_callback_primes(uint64_t start, uint64_t stop, PrimeSieveCallback<uint64_t>* callback, int threads)
{
  ParallelPrimeSieve pps;
  pps.setNumThreads(threads);
  pps.callbackPrimes(start, stop, callback);
}

void parallel_callback_primes(uint64_t start, uint64_t stop, PrimeSieveCallback<uint64_t, int>* callback, int threads)
{
  ParallelPrimeSieve pps;
  pps.setNumThreads(threads);
  pps.callbackPrimes(start, stop, callback);
}

uint64_t parallel_count_primes(uint64_t start, uint64_t stop, int threads)
{
  ParallelPrimeSieve pps;
  pps.setNumThreads(threads);
  return pps.countPrimes(start, stop);
}

uint64_t parallel_count_primes(uint64_t start, uint64_t stop)
{
  return parallel_count_primes(start, stop, MAX_THREADS);
}

uint64_t parallel_count_twins(uint64_t start, uint64_t stop, int threads)
{
  ParallelPrimeSieve pps;
  pps.setNumThreads(threads);
  return pps.countTwins(start, stop);
}

uint64_t parallel_count_twins(uint64_t start, uint64_t stop)
{
  return parallel_count_twins(start, stop, MAX_THREADS);
}

uint64_t parallel_count_triplets(uint64_t start, uint64_t stop, int threads)
{
  ParallelPrimeSieve pps;
  pps.setNumThreads(threads);
  return pps.countTriplets(start, stop);
}

uint64_t parallel_count_triplets(uint64_t start, uint64_t stop)
{
  return parallel_count_triplets(start, stop, MAX_THREADS);
}

uint64_t parallel_count_quadruplets(uint64_t start, uint64_t stop, int threads)
{
  ParallelPrimeSieve pps;
  pps.setNumThreads(threads);
  return pps.countQuadruplets(start, stop);
}

uint64_t parallel_count_quadruplets(uint64_t start, uint64_t stop)
{
  return parallel_count_quadruplets(start, stop, MAX_THREADS);
}

uint64_t parallel_count_quintuplets(uint64_t start, uint64_t stop, int threads)
{
  ParallelPrimeSieve pps;
  pps.setNumThreads(threads);
  return pps.countQuintuplets(start, stop);
}

uint64_t parallel_count_quintuplets(uint64_t start, uint64_t stop)
{
  return parallel_count_quintuplets(start, stop, MAX_THREADS);
}

uint64_t parallel_count_sextuplets(uint64_t start, uint64_t stop, int threads)
{
  ParallelPrimeSieve pps;
  pps.setNumThreads(threads);
  return pps.countSextuplets(start, stop);
}

uint64_t parallel_count_sextuplets(uint64_t start, uint64_t stop)
{
  return parallel_count_sextuplets(start, stop, MAX_THREADS);
}

uint64_t parallel_count_septuplets(uint64_t start, uint64_t stop, int threads)
{
  ParallelPrimeSieve pps;
  pps.setNumThreads(threads);
  return pps.countSeptuplets(start, stop);
}

uint64_t parallel_count_septuplets(uint64_t start, uint64_t stop)
{
  return parallel_count_septuplets(start, stop, MAX_THREADS);
}

uint64_t parallel_nth_prime(uint64_t n, uint64_t start, int threads)
{
  ParallelPrimeSieve pps;
  pps.setNumThreads(threads);
  return pps.nthPrime(n, start);
}

uint64_t parallel_nth_prime(uint64_t n, uint64_t start)
{
  return parallel_nth_prime(n, start, MAX_THREADS);
}

void print_primes(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  ps.printPrimes(start, stop);
}

void print_twins(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  ps.printTwins(start, stop);
}

void print_triplets(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  ps.printTriplets(start, stop);
}

void print_quadruplets(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  ps.printQuadruplets(start, stop);
}

void print_quintuplets(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  ps.printQuintuplets(start, stop);
}

void print_sextuplets(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  ps.printSextuplets(start, stop);
}

void print_septuplets(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  ps.printSeptuplets(start, stop);
}

} // end namespace
