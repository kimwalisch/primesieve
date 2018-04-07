///
/// @file   api-c.cpp
/// @brief  primesieve C API.
///         Contains the implementations of the functions declared
///         in the primesieve.h header file.
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve.h>
#include <primesieve.hpp>
#include <primesieve/PrimeSieve.hpp>
#include <primesieve/ParallelSieve.hpp>
#include <primesieve/malloc_vector.hpp>

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <cerrno>
#include <exception>

using namespace primesieve;

namespace {

template <typename T>
void* primes_helper(uint64_t start, uint64_t stop, size_t* size)
{
  try
  {
    malloc_vector<T> primes;
    store_primes(start, stop, primes);

    if (size)
      *size = primes.size();

    primes.disable_free();
    return (void*) primes.data();
  }
  catch (std::exception&)
  {
    errno = EDOM;
    if (size)
      *size = 0;
  }

  return NULL;
}

template <typename T>
void* n_primes_helper(uint64_t n, uint64_t start)
{
  try
  {
    malloc_vector<T> primes;
    store_n_primes(n, start, primes);
    primes.disable_free();
    return (void*) primes.data();
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }

  return NULL;
}

} // namespace

void* primesieve_generate_primes(uint64_t start, uint64_t stop, size_t* size, int type)
{
  switch (type)
  {
    case SHORT_PRIMES:     return primes_helper<short>(start, stop, size);
    case USHORT_PRIMES:    return primes_helper<unsigned short>(start, stop, size);
    case INT_PRIMES:       return primes_helper<int>(start, stop, size);
    case UINT_PRIMES:      return primes_helper<unsigned int>(start, stop, size);
    case LONG_PRIMES:      return primes_helper<long>(start, stop, size);
    case ULONG_PRIMES:     return primes_helper<unsigned long>(start, stop, size);
    case LONGLONG_PRIMES:  return primes_helper<long long>(start, stop, size);
    case ULONGLONG_PRIMES: return primes_helper<unsigned long long>(start, stop, size);
    case INT16_PRIMES:     return primes_helper<int16_t>(start, stop, size);
    case UINT16_PRIMES:    return primes_helper<uint16_t>(start, stop, size);
    case INT32_PRIMES:     return primes_helper<int32_t>(start, stop, size);
    case UINT32_PRIMES:    return primes_helper<uint32_t>(start, stop, size);
    case INT64_PRIMES:     return primes_helper<int64_t>(start, stop, size);
    case UINT64_PRIMES:    return primes_helper<uint64_t>(start, stop, size);
  }
  errno = EDOM;
  if (size)
    *size = 0;
  return NULL;
}

void* primesieve_generate_n_primes(uint64_t n, uint64_t start, int type)
{
  switch (type)
  {
    case SHORT_PRIMES:     return n_primes_helper<short>(n, start);
    case USHORT_PRIMES:    return n_primes_helper<unsigned short>(n, start);
    case INT_PRIMES:       return n_primes_helper<int>(n, start);
    case UINT_PRIMES:      return n_primes_helper<unsigned int>(n, start);
    case LONG_PRIMES:      return n_primes_helper<long>(n, start);
    case ULONG_PRIMES:     return n_primes_helper<unsigned long>(n, start);
    case LONGLONG_PRIMES:  return n_primes_helper<long long>(n, start);
    case ULONGLONG_PRIMES: return n_primes_helper<unsigned long long>(n, start);
    case INT16_PRIMES:     return n_primes_helper<int16_t>(n, start);
    case UINT16_PRIMES:    return n_primes_helper<uint16_t>(n, start);
    case INT32_PRIMES:     return n_primes_helper<int32_t>(n, start);
    case UINT32_PRIMES:    return n_primes_helper<uint32_t>(n, start);
    case INT64_PRIMES:     return n_primes_helper<int64_t>(n, start);
    case UINT64_PRIMES:    return n_primes_helper<uint64_t>(n, start);
  }
  errno = EDOM;
  return NULL;
}

void primesieve_free(void* primes)
{
  free(primes);
}

uint64_t primesieve_nth_prime(int64_t n, uint64_t start)
{
  try
  {
    ParallelSieve ps;
    ps.setSieveSize(get_sieve_size());
    ps.setNumThreads(get_num_threads());
    return ps.nthPrime(n, start);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
  return PRIMESIEVE_ERROR;
}

uint64_t primesieve_count_primes(uint64_t start, uint64_t stop)
{
  try
  {
    ParallelSieve ps;
    ps.setSieveSize(get_sieve_size());
    ps.setNumThreads(get_num_threads());
    return ps.countPrimes(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
  return PRIMESIEVE_ERROR;
}

uint64_t primesieve_count_twins(uint64_t start, uint64_t stop)
{
  try
  {
    ParallelSieve ps;
    ps.setSieveSize(get_sieve_size());
    ps.setNumThreads(get_num_threads());
    return ps.countTwins(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
  return PRIMESIEVE_ERROR;
}

uint64_t primesieve_count_triplets(uint64_t start, uint64_t stop)
{
  try
  {
    ParallelSieve ps;
    ps.setSieveSize(get_sieve_size());
    ps.setNumThreads(get_num_threads());
    return ps.countTriplets(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
  return PRIMESIEVE_ERROR;
}

uint64_t primesieve_count_quadruplets(uint64_t start, uint64_t stop)
{
  try
  {
    ParallelSieve ps;
    ps.setSieveSize(get_sieve_size());
    ps.setNumThreads(get_num_threads());
    return ps.countQuadruplets(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
  return PRIMESIEVE_ERROR;
}

uint64_t primesieve_count_quintuplets(uint64_t start, uint64_t stop)
{
  try
  {
    ParallelSieve ps;
    ps.setSieveSize(get_sieve_size());
    ps.setNumThreads(get_num_threads());
    return ps.countQuintuplets(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
  return PRIMESIEVE_ERROR;
}

uint64_t primesieve_count_sextuplets(uint64_t start, uint64_t stop)
{
  try
  {
    ParallelSieve ps;
    ps.setSieveSize(get_sieve_size());
    ps.setNumThreads(get_num_threads());
    return ps.countSextuplets(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
  return PRIMESIEVE_ERROR;
}

void primesieve_print_primes(uint64_t start, uint64_t stop)
{
  try
  {
    PrimeSieve ps;
    ps.setSieveSize(get_sieve_size());
    ps.printPrimes(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
}

void primesieve_print_twins(uint64_t start, uint64_t stop)
{
  try
  {
    PrimeSieve ps;
    ps.setSieveSize(get_sieve_size());
    ps.printTwins(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
}

void primesieve_print_triplets(uint64_t start, uint64_t stop)
{
  try
  {
    PrimeSieve ps;
    ps.setSieveSize(get_sieve_size());
    ps.printTriplets(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
}

void primesieve_print_quadruplets(uint64_t start, uint64_t stop)
{
  try
  {
    PrimeSieve ps;
    ps.setSieveSize(get_sieve_size());
    ps.printQuadruplets(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
}

void primesieve_print_quintuplets(uint64_t start, uint64_t stop)
{
  try
  {
    PrimeSieve ps;
    ps.setSieveSize(get_sieve_size());
    ps.printQuintuplets(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
}

void primesieve_print_sextuplets(uint64_t start, uint64_t stop)
{
  try
  {
    PrimeSieve ps;
    ps.setSieveSize(get_sieve_size());
    ps.printSextuplets(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
}

int primesieve_get_sieve_size()
{
  return get_sieve_size();
}

int primesieve_get_num_threads()
{
  return get_num_threads();
}

void primesieve_set_sieve_size(int sieve_size)
{
  set_sieve_size(sieve_size);
}

void primesieve_set_num_threads(int num_threads)
{
  set_num_threads(num_threads);
}

uint64_t primesieve_get_max_stop()
{
  return get_max_stop();
}

const char* primesieve_version()
{
  return PRIMESIEVE_VERSION;
}
