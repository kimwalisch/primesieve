///
/// @file   primesieve-api-c.cpp
/// @brief  Contains the implementations of the functions declared in
///         the primesieve.h header file.
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve.h>
#include <primesieve.hpp>
#include <primesieve/soe/PrimeSieve.hpp>
#include <primesieve/soe/ParallelPrimeSieve.hpp>
#include <primesieve/soe/cancel_callback.hpp>
#include <primesieve/soe/callback_t.hpp>

#include <stdint.h>
#include <stddef.h>
#include <exception>
#include <limits>
#include <cassert>
#include <cerrno>

namespace primesieve
{

uint64_t max_stop();
bool test();

}

//////////////////////////////////////////////////////////////////////
//                    Internal helper functions
//////////////////////////////////////////////////////////////////////

namespace
{

const int BUFFER_BYTES = 128;

/// This is the C array's memory layout:
/// array[ 0] = first prime.
/// array[-1] = memory address of corresponding std::vector object.
/// array[-2] = integer type, e.g. INT_PRIMES.
///
template <typename T>
void* generate_primes_helper(uint64_t start, uint64_t stop, size_t* size, int type)
{
  std::vector<T>& primes = *(new std::vector<T>);
  try
  {
    assert(BUFFER_BYTES % sizeof(T) == 0);
    size_t index_c = BUFFER_BYTES / sizeof(T);
    primes.resize(index_c, 0);
    uintptr_t* primes_c = reinterpret_cast<uintptr_t*>(&primes[index_c]);
    primes_c[-1] = reinterpret_cast<uintptr_t>(&primes);
    primes_c[-2] = type;
    primesieve::generate_primes(start, stop, &primes);
    if (size)
      *size = primes.size() - index_c;
    return reinterpret_cast<void*>(primes_c);
  }
  catch (std::exception&)
  {
    errno = EDOM;
    if (size)
      *size = 0;
    delete &primes;
  }
  return NULL;
}

/// This is the C array's memory layout:
/// array[ 0] = first prime.
/// array[-1] = memory address of corresponding std::vector object.
/// array[-2] = integer type, e.g. INT_PRIMES.
///
template <typename T>
void* generate_n_primes_helper(uint64_t n, uint64_t start, int type)
{
  std::vector<T>& primes = *(new std::vector<T>);
  try
  {
    assert(BUFFER_BYTES % sizeof(T) == 0);
    size_t index_c = BUFFER_BYTES / sizeof(T);
    primes.resize(index_c, 0);
    uintptr_t* primes_c = reinterpret_cast<uintptr_t*>(&primes[index_c]);
    primes_c[-1] = reinterpret_cast<uintptr_t>(&primes);
    primes_c[-2] = type;
    primesieve::generate_n_primes(n, start, &primes);
    return reinterpret_cast<void*>(primes_c);
  }
  catch (std::exception&)
  {
    errno = EDOM;
    delete &primes;
  }
  return NULL;
}

} // namespace

/// All C API functions declared in primesieve.h
/// have extern "C" linkage.
extern "C"
{

//////////////////////////////////////////////////////////////////////
//                    Return an array of primes
//////////////////////////////////////////////////////////////////////

void* generate_primes(uint64_t start, uint64_t stop, size_t* size, int type)
{
  switch (type)
  {
    case SHORT_PRIMES:     return generate_primes_helper<short>(start, stop, size, type);
    case USHORT_PRIMES:    return generate_primes_helper<unsigned short>(start, stop, size, type);
    case INT_PRIMES:       return generate_primes_helper<int>(start, stop, size, type);
    case UINT_PRIMES:      return generate_primes_helper<unsigned int>(start, stop, size, type);
    case LONG_PRIMES:      return generate_primes_helper<long>(start, stop, size, type);
    case ULONG_PRIMES:     return generate_primes_helper<unsigned long>(start, stop, size, type);
    case LONGLONG_PRIMES:  return generate_primes_helper<long long>(start, stop, size, type);
    case ULONGLONG_PRIMES: return generate_primes_helper<unsigned long long>(start, stop, size, type);
    case INT16_PRIMES:     return generate_primes_helper<int16_t>(start, stop, size, type);
    case UINT16_PRIMES:    return generate_primes_helper<uint16_t>(start, stop, size, type);
    case INT32_PRIMES:     return generate_primes_helper<int32_t>(start, stop, size, type);
    case UINT32_PRIMES:    return generate_primes_helper<uint32_t>(start, stop, size, type);
    case INT64_PRIMES:     return generate_primes_helper<int64_t>(start, stop, size, type);
    case UINT64_PRIMES:    return generate_primes_helper<uint64_t>(start, stop, size, type);
  }
  errno = EDOM;
  if (size)
    *size = 0;
  return NULL;
}

void* generate_n_primes(uint64_t n, uint64_t start, int type)
{
  switch (type)
  {
    case SHORT_PRIMES:     return generate_n_primes_helper<short>(n, start, type);
    case USHORT_PRIMES:    return generate_n_primes_helper<unsigned short>(n, start, type);
    case INT_PRIMES:       return generate_n_primes_helper<int>(n, start, type);
    case UINT_PRIMES:      return generate_n_primes_helper<unsigned int>(n, start, type);
    case LONG_PRIMES:      return generate_n_primes_helper<long>(n, start, type);
    case ULONG_PRIMES:     return generate_n_primes_helper<unsigned long>(n, start, type);
    case LONGLONG_PRIMES:  return generate_n_primes_helper<long long>(n, start, type);
    case ULONGLONG_PRIMES: return generate_n_primes_helper<unsigned long long>(n, start, type);
    case INT16_PRIMES:     return generate_n_primes_helper<int16_t>(n, start, type);
    case UINT16_PRIMES:    return generate_n_primes_helper<uint16_t>(n, start, type);
    case INT32_PRIMES:     return generate_n_primes_helper<int32_t>(n, start, type);
    case UINT32_PRIMES:    return generate_n_primes_helper<uint32_t>(n, start, type);
    case INT64_PRIMES:     return generate_n_primes_helper<int64_t>(n, start, type);
    case UINT64_PRIMES:    return generate_n_primes_helper<uint64_t>(n, start, type);
  }
  errno = EDOM;
  return NULL;
}

void primesieve_free(void* array)
{
  if (array)
  {
    uintptr_t pimpl = reinterpret_cast<uintptr_t*>(array)[-1];
    uintptr_t type  = reinterpret_cast<uintptr_t*>(array)[-2];
    switch (type)
    {
      case SHORT_PRIMES:     delete reinterpret_cast<std::vector<short>* >(pimpl); break;
      case USHORT_PRIMES:    delete reinterpret_cast<std::vector<unsigned short>* >(pimpl); break;
      case INT_PRIMES:       delete reinterpret_cast<std::vector<int>* >(pimpl); break;
      case UINT_PRIMES:      delete reinterpret_cast<std::vector<unsigned int>* >(pimpl); break;
      case LONG_PRIMES:      delete reinterpret_cast<std::vector<long>* >(pimpl); break;
      case ULONG_PRIMES:     delete reinterpret_cast<std::vector<unsigned long>* >(pimpl); break;
      case LONGLONG_PRIMES:  delete reinterpret_cast<std::vector<long long>* >(pimpl); break;
      case ULONGLONG_PRIMES: delete reinterpret_cast<std::vector<unsigned long long>* >(pimpl); break;
      case INT16_PRIMES:     delete reinterpret_cast<std::vector<int16_t>* >(pimpl); break;
      case UINT16_PRIMES:    delete reinterpret_cast<std::vector<uint16_t>* >(pimpl); break;
      case INT32_PRIMES:     delete reinterpret_cast<std::vector<int32_t>* >(pimpl); break;
      case UINT32_PRIMES:    delete reinterpret_cast<std::vector<uint32_t>* >(pimpl); break;
      case INT64_PRIMES:     delete reinterpret_cast<std::vector<int64_t>* >(pimpl); break;
      case UINT64_PRIMES:    delete reinterpret_cast<std::vector<uint64_t>* >(pimpl); break;
      default :              errno = EDOM;
    }
  }
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
    // temporarily cast away extern "C" linkage
    ps.c_callbackPrimes(start, stop, reinterpret_cast<callback_t>(callback));
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
    // temporarily cast away extern "C" linkage
    pps.c_callbackPrimes(start, stop, reinterpret_cast<callback_tn_t>(callback));
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

} // extern "C"
