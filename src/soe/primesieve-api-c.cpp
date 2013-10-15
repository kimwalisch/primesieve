///
/// @file   primesieve-api-c.cpp
/// @brief  Contains the implementations of the functions declared in
///         the primesieve-c.h header file.
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve-c.h>
#include <primesieve/soe/PrimeSieve.hpp>
#include <primesieve/soe/ParallelPrimeSieve.hpp>
#include <primesieve/soe/cancel_callback.hpp>

#include <stdint.h>
#include <exception>
#include <limits>
#include <cerrno>

namespace primesieve
{
  uint64_t max_stop();
  bool test();
}

//////////////////////////////////////////////////////////////////////
//                     Nth prime functions
//////////////////////////////////////////////////////////////////////

uint64_t nth_prime(uint64_t n, uint64_t start)
{
  try
  {
    PrimeSieve ps;
    return ps.nthPrime(n, start);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
  return PRIMESIEVE_ERROR;
}

uint64_t parallel_nth_prime(uint64_t n, uint64_t start, int threads)
{
  try
  {
    ParallelPrimeSieve pps;
    pps.setNumThreads(threads);
    return pps.nthPrime(n, start);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
  return PRIMESIEVE_ERROR;
}

//////////////////////////////////////////////////////////////////////
//                      Count functions
//////////////////////////////////////////////////////////////////////

uint64_t count_primes(uint64_t start, uint64_t stop)
{
  try
  {
    PrimeSieve ps;
    return ps.countPrimes(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
  return PRIMESIEVE_ERROR;
}

uint64_t count_twins(uint64_t start, uint64_t stop)
{
  try
  {
    PrimeSieve ps;
    return ps.countTwins(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
  return PRIMESIEVE_ERROR;
}

uint64_t count_triplets(uint64_t start, uint64_t stop)
{
  try
  {
    PrimeSieve ps;
    return ps.countTriplets(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
  return PRIMESIEVE_ERROR;
}

uint64_t count_quadruplets(uint64_t start, uint64_t stop)
{
  try
  {
    PrimeSieve ps;
    return ps.countQuadruplets(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
  return PRIMESIEVE_ERROR;
}

uint64_t count_quintuplets(uint64_t start, uint64_t stop)
{
  try
  {
    PrimeSieve ps;
    return ps.countQuintuplets(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
  return PRIMESIEVE_ERROR;
}

uint64_t count_sextuplets(uint64_t start, uint64_t stop)
{
  try
  {
    PrimeSieve ps;
    return ps.countSextuplets(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
  return PRIMESIEVE_ERROR;
}

uint64_t count_septuplets(uint64_t start, uint64_t stop)
{
  try
  {
    PrimeSieve ps;
    return ps.countSeptuplets(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
  return PRIMESIEVE_ERROR;
}

//////////////////////////////////////////////////////////////////////
//                   Parallel count functions
//////////////////////////////////////////////////////////////////////

uint64_t parallel_count_primes(uint64_t start, uint64_t stop, int threads)
{
  try
  {
    ParallelPrimeSieve pps;
    pps.setNumThreads(threads);
    return pps.countPrimes(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
  return PRIMESIEVE_ERROR;
}

uint64_t parallel_count_twins(uint64_t start, uint64_t stop, int threads)
{
  try
  {
    ParallelPrimeSieve pps;
    pps.setNumThreads(threads);
    return pps.countTwins(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
  return PRIMESIEVE_ERROR;
}

uint64_t parallel_count_triplets(uint64_t start, uint64_t stop, int threads)
{
  try
  {
    ParallelPrimeSieve pps;
    pps.setNumThreads(threads);
    return pps.countTriplets(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
  return PRIMESIEVE_ERROR;
}

uint64_t parallel_count_quadruplets(uint64_t start, uint64_t stop, int threads)
{
  try
  {
    ParallelPrimeSieve pps;
    pps.setNumThreads(threads);
    return pps.countQuadruplets(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
  return PRIMESIEVE_ERROR;
}

uint64_t parallel_count_quintuplets(uint64_t start, uint64_t stop, int threads)
{
  try
  {
    ParallelPrimeSieve pps;
    pps.setNumThreads(threads);
    return pps.countQuintuplets(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
  return PRIMESIEVE_ERROR;
}

uint64_t parallel_count_sextuplets(uint64_t start, uint64_t stop, int threads)
{
  try
  {
    ParallelPrimeSieve pps;
    pps.setNumThreads(threads);
    return pps.countSextuplets(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
  return PRIMESIEVE_ERROR;
}

uint64_t parallel_count_septuplets(uint64_t start, uint64_t stop, int threads)
{
  try
  {
    ParallelPrimeSieve pps;
    pps.setNumThreads(threads);
    return pps.countSeptuplets(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
  return PRIMESIEVE_ERROR;
}

//////////////////////////////////////////////////////////////////////
//                      Print functions
//////////////////////////////////////////////////////////////////////

void print_primes(uint64_t start, uint64_t stop)
{
  try
  {
    PrimeSieve ps;
    ps.printPrimes(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
}

void print_twins(uint64_t start, uint64_t stop)
{
  try
  {
    PrimeSieve ps;
    ps.printTwins(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
}

void print_triplets(uint64_t start, uint64_t stop)
{
  try
  {
    PrimeSieve ps;
    ps.printTriplets(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
}

void print_quadruplets(uint64_t start, uint64_t stop)
{
  try
  {
    PrimeSieve ps;
    ps.printQuadruplets(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
}

void print_quintuplets(uint64_t start, uint64_t stop)
{
  try
  {
    PrimeSieve ps;
    ps.printQuintuplets(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
}

void print_sextuplets(uint64_t start, uint64_t stop)
{
  try
  {
    PrimeSieve ps;
    ps.printSextuplets(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
}

void print_septuplets(uint64_t start, uint64_t stop)
{
  try
  {
    PrimeSieve ps;
    ps.printSeptuplets(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
}

//////////////////////////////////////////////////////////////////////
//                      Callback functions
//////////////////////////////////////////////////////////////////////

void callback_primes(uint64_t start, uint64_t stop, void (*callback)(uint64_t))
{
  try
  {
    PrimeSieve ps;
    ps.callbackPrimes(start, stop, callback);
  }
  catch (std::exception&)
  {
    errno = EDOM;
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
  catch (std::exception&)
  {
    errno = EDOM;
  }
}

//////////////////////////////////////////////////////////////////////
//                        Other functions
//////////////////////////////////////////////////////////////////////

uint64_t max_stop()
{
  return primesieve::max_stop();
}

int primesieve_test()
{
  return (primesieve::test() == true) ? 1 : 0;
}
