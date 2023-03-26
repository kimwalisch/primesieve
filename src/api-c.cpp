///
/// @file   api-c.cpp
/// @brief  primesieve C API.
///         Contains the implementations of the functions declared
///         in the primesieve.h header file.
///
/// Copyright (C) 2021 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve.h>
#include <primesieve.hpp>
#include <primesieve/malloc_vector.hpp>

#include <stdint.h>
#include <cstdlib>
#include <cstddef>
#include <cerrno>
#include <exception>
#include <iostream>

using std::size_t;
using namespace primesieve;

namespace {

template <typename T>
void* get_primes(uint64_t start, uint64_t stop, size_t* size)
{
  try
  {
    malloc_vector<T> primes;
    store_primes(start, stop, primes);
    if (size)
      *size = primes.size();

    return primes.release();
  }
  catch (const std::exception& e)
  {
    if (size)
      *size = 0;

    std::cerr << "primesieve_generate_primes: " << e.what() << std::endl;
    errno = EDOM;
    return nullptr;
  }
}

template <typename T>
void* get_n_primes(uint64_t n, uint64_t start)
{
  try
  {
    malloc_vector<T> primes;
    store_n_primes(n, start, primes);
    return primes.release();
  }
  catch (const std::exception& e)
  {
    std::cerr << "primesieve_generate_n_primes: " << e.what() << std::endl;
    errno = EDOM;
    return nullptr;
  }
}

} // namespace

void* primesieve_generate_primes(uint64_t start, uint64_t stop, size_t* size, int type)
{
  switch (type)
  {
    case SHORT_PRIMES:     return get_primes<short>(start, stop, size);
    case USHORT_PRIMES:    return get_primes<unsigned short>(start, stop, size);
    case INT_PRIMES:       return get_primes<int>(start, stop, size);
    case UINT_PRIMES:      return get_primes<unsigned int>(start, stop, size);
    case LONG_PRIMES:      return get_primes<long>(start, stop, size);
    case ULONG_PRIMES:     return get_primes<unsigned long>(start, stop, size);
    case LONGLONG_PRIMES:  return get_primes<long long>(start, stop, size);
    case ULONGLONG_PRIMES: return get_primes<unsigned long long>(start, stop, size);
    case INT16_PRIMES:     return get_primes<int16_t>(start, stop, size);
    case UINT16_PRIMES:    return get_primes<uint16_t>(start, stop, size);
    case INT32_PRIMES:     return get_primes<int32_t>(start, stop, size);
    case UINT32_PRIMES:    return get_primes<uint32_t>(start, stop, size);
    case INT64_PRIMES:     return get_primes<int64_t>(start, stop, size);
    case UINT64_PRIMES:    return get_primes<uint64_t>(start, stop, size);
  }

  if (size)
    *size = 0;

  std::cerr << "primesieve_generate_primes: Invalid type parameter!" << std::endl;
  errno = EDOM;
  return nullptr;
}

void* primesieve_generate_n_primes(uint64_t n, uint64_t start, int type)
{
  switch (type)
  {
    case SHORT_PRIMES:     return get_n_primes<short>(n, start);
    case USHORT_PRIMES:    return get_n_primes<unsigned short>(n, start);
    case INT_PRIMES:       return get_n_primes<int>(n, start);
    case UINT_PRIMES:      return get_n_primes<unsigned int>(n, start);
    case LONG_PRIMES:      return get_n_primes<long>(n, start);
    case ULONG_PRIMES:     return get_n_primes<unsigned long>(n, start);
    case LONGLONG_PRIMES:  return get_n_primes<long long>(n, start);
    case ULONGLONG_PRIMES: return get_n_primes<unsigned long long>(n, start);
    case INT16_PRIMES:     return get_n_primes<int16_t>(n, start);
    case UINT16_PRIMES:    return get_n_primes<uint16_t>(n, start);
    case INT32_PRIMES:     return get_n_primes<int32_t>(n, start);
    case UINT32_PRIMES:    return get_n_primes<uint32_t>(n, start);
    case INT64_PRIMES:     return get_n_primes<int64_t>(n, start);
    case UINT64_PRIMES:    return get_n_primes<uint64_t>(n, start);
  }

  std::cerr << "primesieve_generate_n_primes: Invalid type parameter!" << std::endl;
  errno = EDOM;
  return nullptr;
}

void primesieve_free(void* primes)
{
  free(primes);
}

uint64_t primesieve_nth_prime(int64_t n, uint64_t start)
{
  try
  {
    return nth_prime(n, start);
  }
  catch (const std::exception& e)
  {
    std::cerr << "primesieve_nth_prime: " << e.what() << std::endl;
    errno = EDOM;
    return PRIMESIEVE_ERROR;
  }
}

uint64_t primesieve_count_primes(uint64_t start, uint64_t stop)
{
  try
  {
    return count_primes(start, stop);
  }
  catch (const std::exception& e)
  {
    std::cerr << "primesieve_count_primes: " << e.what() << std::endl;
    errno = EDOM;
    return PRIMESIEVE_ERROR;
  }
}

uint64_t primesieve_count_twins(uint64_t start, uint64_t stop)
{
  try
  {
    return count_twins(start, stop);
  }
  catch (const std::exception& e)
  {
    std::cerr << "primesieve_count_twins: " << e.what() << std::endl;
    errno = EDOM;
    return PRIMESIEVE_ERROR;
  }
}

uint64_t primesieve_count_triplets(uint64_t start, uint64_t stop)
{
  try
  {
    return count_triplets(start, stop);
  }
  catch (const std::exception& e)
  {
    std::cerr << "primesieve_count_triplets: " << e.what() << std::endl;
    errno = EDOM;
    return PRIMESIEVE_ERROR;
  }
}

uint64_t primesieve_count_quadruplets(uint64_t start, uint64_t stop)
{
  try
  {
    return count_quadruplets(start, stop);
  }
  catch (const std::exception& e)
  {
    std::cerr << "primesieve_count_quadruplets: " << e.what() << std::endl;
    errno = EDOM;
    return PRIMESIEVE_ERROR;
  }
}

uint64_t primesieve_count_quintuplets(uint64_t start, uint64_t stop)
{
  try
  {
    return count_quintuplets(start, stop);
  }
  catch (const std::exception& e)
  {
    std::cerr << "primesieve_count_quintuplets: " << e.what() << std::endl;
    errno = EDOM;
    return PRIMESIEVE_ERROR;
  }
}

uint64_t primesieve_count_sextuplets(uint64_t start, uint64_t stop)
{
  try
  {
    return count_sextuplets(start, stop);
  }
  catch (const std::exception& e)
  {
    std::cerr << "primesieve_count_sextuplets: " << e.what() << std::endl;
    errno = EDOM;
    return PRIMESIEVE_ERROR;
  }
}

void primesieve_print_primes(uint64_t start, uint64_t stop)
{
  try
  {
    print_primes(start, stop);
  }
  catch (const std::exception& e)
  {
    std::cerr << "primesieve_print_primes: " << e.what() << std::endl;
    errno = EDOM;
  }
}

void primesieve_print_twins(uint64_t start, uint64_t stop)
{
  try
  {
    print_twins(start, stop);
  }
  catch (const std::exception& e)
  {
    std::cerr << "primesieve_print_twins: " << e.what() << std::endl;
    errno = EDOM;
  }
}

void primesieve_print_triplets(uint64_t start, uint64_t stop)
{
  try
  {
    print_triplets(start, stop);
  }
  catch (const std::exception& e)
  {
    std::cerr << "primesieve_print_triplets: " << e.what() << std::endl;
    errno = EDOM;
  }
}

void primesieve_print_quadruplets(uint64_t start, uint64_t stop)
{
  try
  {
    print_quadruplets(start, stop);
  }
  catch (const std::exception& e)
  {
    std::cerr << "primesieve_print_quadruplets: " << e.what() << std::endl;
    errno = EDOM;
  }
}

void primesieve_print_quintuplets(uint64_t start, uint64_t stop)
{
  try
  {
    print_quintuplets(start, stop);
  }
  catch (const std::exception& e)
  {
    std::cerr << "primesieve_print_quintuplets: " << e.what() << std::endl;
    errno = EDOM;
  }
}

void primesieve_print_sextuplets(uint64_t start, uint64_t stop)
{
  try
  {
    print_sextuplets(start, stop);
  }
  catch (const std::exception& e)
  {
    std::cerr << "primesieve_print_sextuplets: " << e.what() << std::endl;
    errno = EDOM;
  }
}

int primesieve_get_sieve_size(void)
{
  return get_sieve_size();
}

int primesieve_get_num_threads(void)
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

uint64_t primesieve_get_max_stop(void)
{
  return get_max_stop();
}

const char* primesieve_version(void)
{
  return PRIMESIEVE_VERSION;
}
