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

#include "PrimeSieve.h"
#include "ParallelPrimeSieve.h"
#include "PrimeSieveCallback.h"

#include <stdint.h>

namespace primesieve {

void callback_primes(uint32_t start, uint32_t stop, PrimeSieveCallback<uint32_t>* callback)
{
  PrimeSieve ps;
  ps.generatePrimes(start, stop, callback);
}

void callback_primes(uint64_t start, uint64_t stop, PrimeSieveCallback<uint64_t>* callback)
{
  PrimeSieve ps;
  ps.generatePrimes(start, stop, callback);
}

void callback_primes(uint32_t start, uint32_t stop, int threads, void (*callback)(uint32_t prime))
{
  ParallelPrimeSieve pps;
  pps.setNumThreads(threads);
  pps.generatePrimes(start, stop, callback);
}

void callback_primes(uint64_t start, uint64_t stop, int threads, void (*callback)(uint64_t prime))
{
  ParallelPrimeSieve pps;
  pps.setNumThreads(threads);
  pps.generatePrimes(start, stop, callback);
}

void callback_primes(uint64_t start, uint64_t stop, int threads, void (*callback)(uint64_t prime, int thread_id))
{
  ParallelPrimeSieve pps;
  pps.setNumThreads(threads);
  pps.generatePrimes(start, stop, callback);
}

void callback_primes(uint32_t start, uint32_t stop, int threads, PrimeSieveCallback<uint32_t>* callback)
{
  ParallelPrimeSieve pps;
  pps.setNumThreads(threads);
  pps.generatePrimes(start, stop, callback);
}

void callback_primes(uint64_t start, uint64_t stop, int threads, PrimeSieveCallback<uint64_t>* callback)
{
  ParallelPrimeSieve pps;
  pps.setNumThreads(threads);
  pps.generatePrimes(start, stop, callback);
}

void callback_primes(uint64_t start, uint64_t stop, int threads, PrimeSieveCallback<uint64_t, int>* callback)
{
  ParallelPrimeSieve pps;
  pps.setNumThreads(threads);
  pps.generatePrimes(start, stop, callback);
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

uint64_t count_primes(uint64_t start, uint64_t stop, int threads)
{
  ParallelPrimeSieve pps;
  pps.setNumThreads(threads);
  return pps.countPrimes(start, stop);
}

uint64_t count_twins(uint64_t start, uint64_t stop, int threads)
{
  ParallelPrimeSieve pps;
  pps.setNumThreads(threads);
  return pps.countTwins(start, stop);
}

uint64_t count_triplets(uint64_t start, uint64_t stop, int threads)
{
  ParallelPrimeSieve pps;
  pps.setNumThreads(threads);
  return pps.countTriplets(start, stop);
}

uint64_t count_quadruplets(uint64_t start, uint64_t stop, int threads)
{
  ParallelPrimeSieve pps;
  pps.setNumThreads(threads);
  return pps.countQuadruplets(start, stop);
}

uint64_t count_quintuplets(uint64_t start, uint64_t stop, int threads)
{
  ParallelPrimeSieve pps;
  pps.setNumThreads(threads);
  return pps.countQuintuplets(start, stop);
}

uint64_t count_sextuplets(uint64_t start, uint64_t stop, int threads)
{
  ParallelPrimeSieve pps;
  pps.setNumThreads(threads);
  return pps.countSextuplets(start, stop);
}

uint64_t count_septuplets(uint64_t start, uint64_t stop, int threads)
{
  ParallelPrimeSieve pps;
  pps.setNumThreads(threads);
  return pps.countSeptuplets(start, stop);
}

uint64_t nth_prime(uint64_t n)
{
  PrimeSieve ps;
  return ps.nthPrime(n);
}

uint64_t nth_prime(uint64_t n, int threads)
{
  ParallelPrimeSieve pps;
  pps.setNumThreads(threads);
  return pps.nthPrime(n);
}

uint64_t nth_prime(uint64_t n, uint64_t start, int threads)
{
  ParallelPrimeSieve pps;
  pps.setNumThreads(threads);
  return pps.nthPrime(n, start);
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

void print_primes(uint64_t start, uint64_t stop, int threads)
{
  ParallelPrimeSieve pps;
  pps.setNumThreads(threads);
  pps.printPrimes(start, stop);
}

void print_twins(uint64_t start, uint64_t stop, int threads)
{
  ParallelPrimeSieve pps;
  pps.setNumThreads(threads);
  pps.printTwins(start, stop);
}

void print_triplets(uint64_t start, uint64_t stop, int threads)
{
  ParallelPrimeSieve pps;
  pps.setNumThreads(threads);
  pps.printTriplets(start, stop);
}

void print_quadruplets(uint64_t start, uint64_t stop, int threads)
{
  ParallelPrimeSieve pps;
  pps.setNumThreads(threads);
  pps.printQuadruplets(start, stop);
}

void print_quintuplets(uint64_t start, uint64_t stop, int threads)
{
  ParallelPrimeSieve pps;
  pps.setNumThreads(threads);
  pps.printQuintuplets(start, stop);
}

void print_sextuplets(uint64_t start, uint64_t stop, int threads)
{
  ParallelPrimeSieve pps;
  pps.setNumThreads(threads);
  pps.printSextuplets(start, stop);
}

void print_septuplets(uint64_t start, uint64_t stop, int threads)
{
  ParallelPrimeSieve pps;
  pps.setNumThreads(threads);
  pps.printSeptuplets(start, stop);
}

} // end namespace
