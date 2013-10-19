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
#include <primesieve/soe/c_callback.h>

#include <stdint.h>
#include <stddef.h>
#include <exception>
#include <limits>
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

/// This is the C array's memory layout:
/// array[-2] = memory address of corresponding std::vector object.
/// array[-1] = integer type, e.g. INT_PRIMES.
/// array[ 0] = first prime.
///
template <typename T>
void* generate_primes_helper(uint64_t start, uint64_t stop, size_t* size, int type)
{
  std::vector<T>& primes = *(new std::vector<T>);
  try
  {
    primes.resize(128 / sizeof(T), 0);
    reinterpret_cast<uintptr_t*>(reinterpret_cast<uint8_t*>(&primes[0]) + 128)[-1] = type;
    reinterpret_cast<uintptr_t*>(reinterpret_cast<uint8_t*>(&primes[0]) + 128)[-2] = reinterpret_cast<uintptr_t>(&primes);
    primesieve::generate_primes(start, stop, &primes);
    *size = primes.size() - 128 / sizeof(T);
    return reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(&primes[0]) + 128);
  }
  catch (std::exception&)
  {
    errno = EDOM;
    *size = 0;
    delete &primes;
  }
  return NULL;
}

/// This is the C array's memory layout:
/// array[-2] = memory address of corresponding std::vector object.
/// array[-1] = integer type, e.g. INT_PRIMES.
/// array[ 0] = first prime.
///
template <typename T>
void* generate_n_primes_helper(uint64_t n, uint64_t start, int type)
{
  std::vector<T>& primes = *(new std::vector<T>);
  try
  {
    primes.resize(128 / sizeof(T), 0);
    reinterpret_cast<uintptr_t*>(reinterpret_cast<uint8_t*>(&primes[0]) + 128)[-1] = type;
    reinterpret_cast<uintptr_t*>(reinterpret_cast<uint8_t*>(&primes[0]) + 128)[-2] = reinterpret_cast<uintptr_t>(&primes);
    primesieve::generate_n_primes(n, start, &primes);
    return reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(&primes[0]) + 128);
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
  uintptr_t type  = reinterpret_cast<uintptr_t*>(array)[-1];
  uintptr_t pimpl = reinterpret_cast<uintptr_t*>(array)[-2];
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

void callback_primes(uint64_t start, uint64_t stop, c_callback_t callback)
{
  try
  {
    PrimeSieve ps;
    ps.c_callbackPrimes(start, stop, callback);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
}

void parallel_callback_primes(uint64_t start, uint64_t stop, c_callback_tn_t callback, int threads)
{
  try
  {
    ParallelPrimeSieve pps;
    pps.setNumThreads(threads);
    pps.c_callbackPrimes(start, stop, callback);
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
