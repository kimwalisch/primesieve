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
#include <iostream>
#include <exception>

namespace primesieve {

//////////////////////////////////////////////////////////////////////
//                Functions with C++ linkage
//////////////////////////////////////////////////////////////////////

uint64_t nth_prime(uint64_t n)
{
  uint64_t result = 0;
  try
  {
    result = nth_prime(n, 0);
  }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
  return result;
}

uint64_t parallel_nth_prime(uint64_t n, uint64_t start)
{
  uint64_t result = 0;
  try
  {
    result = parallel_nth_prime(n, start, MAX_THREADS);
  }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
  return result;
}

uint64_t parallel_count_primes(uint64_t start, uint64_t stop)
{
  uint64_t result = 0;
  try
  {
    result = parallel_count_primes(start, stop, MAX_THREADS);
  }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
  return result;
}

uint64_t parallel_count_twins(uint64_t start, uint64_t stop)
{
  uint64_t result = 0;
  try
  {
    result = parallel_count_twins(start, stop, MAX_THREADS);
  }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
  return result;
}

uint64_t parallel_count_triplets(uint64_t start, uint64_t stop)
{
  uint64_t result = 0;
  try
  {
    result = parallel_count_triplets(start, stop, MAX_THREADS);
  }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
  return result;
}

uint64_t parallel_count_quadruplets(uint64_t start, uint64_t stop)
{
  uint64_t result = 0;
  try
  {
    result = parallel_count_quadruplets(start, stop, MAX_THREADS);
  }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
  return result;
}

uint64_t parallel_count_quintuplets(uint64_t start, uint64_t stop)
{
  uint64_t result = 0;
  try
  {
    result = parallel_count_quintuplets(start, stop, MAX_THREADS);
  }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
  return result;
}

uint64_t parallel_count_sextuplets(uint64_t start, uint64_t stop)
{
  uint64_t result = 0;
  try
  {
    result = parallel_count_sextuplets(start, stop, MAX_THREADS);
  }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
  return result;
}

uint64_t parallel_count_septuplets(uint64_t start, uint64_t stop)
{
  uint64_t result = 0;
  try
  {
    result = parallel_count_septuplets(start, stop, MAX_THREADS);
  }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
  return result;
}

void callback_primes(uint64_t start, uint64_t stop, PrimeSieveCallback<uint64_t>* callback)
{
  try
  {
    PrimeSieve ps;
    ps.callbackPrimes(start, stop, callback);
  }
  catch (cancel_callback&)
  { }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
}

void parallel_callback_primes(uint64_t start, uint64_t stop, PrimeSieveCallback<uint64_t>* callback, int threads)
{
  try
  {
    ParallelPrimeSieve pps;
    pps.setNumThreads(threads);
    pps.callbackPrimes(start, stop, callback);
  }
  catch (cancel_callback&)
  { }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
}

void parallel_callback_primes(uint64_t start, uint64_t stop, PrimeSieveCallback<uint64_t, int>* callback, int threads)
{
  try
  {
    ParallelPrimeSieve pps;
    pps.setNumThreads(threads);
    pps.callbackPrimes(start, stop, callback);
  }
  catch (cancel_callback&)
  { }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
}

//////////////////////////////////////////////////////////////////////
//               Functions with extern "C" linkage
//////////////////////////////////////////////////////////////////////

uint64_t nth_prime(uint64_t n, uint64_t start)
{
  uint64_t result = 0;
  try
  {
    PrimeSieve ps;
    result = ps.nthPrime(n, start);
  }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
  return result;
}

uint64_t count_primes(uint64_t start, uint64_t stop)
{
  uint64_t result = 0;
  try
  {
    PrimeSieve ps;
    result = ps.countPrimes(start, stop);
  }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
  return result;
}

uint64_t count_twins(uint64_t start, uint64_t stop)
{
  uint64_t result = 0;
  try
  {
    PrimeSieve ps;
    result = ps.countTwins(start, stop);
  }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
  return result;
}

uint64_t count_triplets(uint64_t start, uint64_t stop)
{
  uint64_t result = 0;
  try
  {
    PrimeSieve ps;
    result = ps.countTriplets(start, stop);
  }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
  return result;
}

uint64_t count_quadruplets(uint64_t start, uint64_t stop)
{
  uint64_t result = 0;
  try
  {
    PrimeSieve ps;
    result = ps.countQuadruplets(start, stop);
  }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
  return result;
}

uint64_t count_quintuplets(uint64_t start, uint64_t stop)
{
  uint64_t result = 0;
  try
  {
    PrimeSieve ps;
    result = ps.countQuintuplets(start, stop);
  }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
  return result;
}

uint64_t count_sextuplets(uint64_t start, uint64_t stop)
{
  uint64_t result = 0;
  try
  {
    PrimeSieve ps;
    result = ps.countSextuplets(start, stop);
  }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
  return result;
}

uint64_t count_septuplets(uint64_t start, uint64_t stop)
{
  uint64_t result = 0;
  try
  {
    PrimeSieve ps;
    result = ps.countSeptuplets(start, stop);
  }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
  return result;
}

void print_primes(uint64_t start, uint64_t stop)
{
  try
  {
    PrimeSieve ps;
    ps.printPrimes(start, stop);
  }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
}

void print_twins(uint64_t start, uint64_t stop)
{
  try
  {
    PrimeSieve ps;
    ps.printTwins(start, stop);
  }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
}

void print_triplets(uint64_t start, uint64_t stop)
{
  try
  {
    PrimeSieve ps;
    ps.printTriplets(start, stop);
  }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
}

void print_quadruplets(uint64_t start, uint64_t stop)
{
  try
  {
    PrimeSieve ps;
    ps.printQuadruplets(start, stop);
  }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
}

void print_quintuplets(uint64_t start, uint64_t stop)
{
  try
  {
    PrimeSieve ps;
    ps.printQuintuplets(start, stop);
  }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
}

void print_sextuplets(uint64_t start, uint64_t stop)
{
  try
  {
    PrimeSieve ps;
    ps.printSextuplets(start, stop);
  }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
}

void print_septuplets(uint64_t start, uint64_t stop)
{
  try
  {
    PrimeSieve ps;
    ps.printSeptuplets(start, stop);
  }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
}

void callback_primes(uint64_t start, uint64_t stop, void (*callback)(uint64_t))
{
  try
  {
    PrimeSieve ps;
    ps.callbackPrimes(start, stop, callback);
  }
  catch (cancel_callback&)
  { }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
}

void parallel_callback_primes(uint64_t start, uint64_t stop, void (*callback)(uint64_t), int threads)
{
  try
  {
    ParallelPrimeSieve pps;
    pps.setNumThreads(threads);
    pps.callbackPrimes(start, stop, callback);
  }
  catch (cancel_callback&)
  { }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
}

void parallel_callback_primes(uint64_t start, uint64_t stop, void (*callback)(uint64_t, int), int threads)
{
  try
  {
    ParallelPrimeSieve pps;
    pps.setNumThreads(threads);
    pps.callbackPrimes(start, stop, callback);
  }
  catch (cancel_callback&)
  { }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
}

void parallel_callback_primes(uint64_t start, uint64_t stop, void (*callback)(uint64_t, int))
{
  try
  {
    ParallelPrimeSieve pps;
    pps.setNumThreads(MAX_THREADS);
    pps.callbackPrimes(start, stop, callback);
  }
  catch (cancel_callback&)
  { }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
}

uint64_t parallel_count_primes(uint64_t start, uint64_t stop, int threads)
{
  uint64_t result = 0;
  try
  {
    ParallelPrimeSieve pps;
    pps.setNumThreads(threads);
    result = pps.countPrimes(start, stop);
  }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
  return result;
}

uint64_t parallel_count_twins(uint64_t start, uint64_t stop, int threads)
{
  uint64_t result = 0;
  try
  {
    ParallelPrimeSieve pps;
    pps.setNumThreads(threads);
    result = pps.countTwins(start, stop);
  }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
  return result;
}

uint64_t parallel_count_triplets(uint64_t start, uint64_t stop, int threads)
{
  uint64_t result = 0;
  try
  {
    ParallelPrimeSieve pps;
    pps.setNumThreads(threads);
    result = pps.countTriplets(start, stop);
  }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
  return result;
}

uint64_t parallel_count_quadruplets(uint64_t start, uint64_t stop, int threads)
{
  uint64_t result = 0;
  try
  {
    ParallelPrimeSieve pps;
    pps.setNumThreads(threads);
    result = pps.countQuadruplets(start, stop);
  }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
  return result;
}

uint64_t parallel_count_quintuplets(uint64_t start, uint64_t stop, int threads)
{
  uint64_t result = 0;
  try
  {
    ParallelPrimeSieve pps;
    pps.setNumThreads(threads);
    result = pps.countQuintuplets(start, stop);
  }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
  return result;
}

uint64_t parallel_count_sextuplets(uint64_t start, uint64_t stop, int threads)
{
  uint64_t result = 0;
  try
  {
    ParallelPrimeSieve pps;
    pps.setNumThreads(threads);
    result = pps.countSextuplets(start, stop);
  }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
  return result;
}

uint64_t parallel_count_septuplets(uint64_t start, uint64_t stop, int threads)
{
  uint64_t result = 0;
  try
  {
    ParallelPrimeSieve pps;
    pps.setNumThreads(threads);
    result = pps.countSeptuplets(start, stop);
  }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
  return result;
}

uint64_t parallel_nth_prime(uint64_t n, uint64_t start, int threads)
{
  uint64_t result = 0;
  try
  {
    ParallelPrimeSieve pps;
    pps.setNumThreads(threads);
    result = pps.nthPrime(n, start);
  }
  catch (std::exception& e)
  {
    std::cerr << "primesieve error: " << e.what() << std::endl;
  }
  return result;
}

uint64_t max_stop()
{
  return soe::PrimeFinder::getMaxStop();
}

} // end namespace
