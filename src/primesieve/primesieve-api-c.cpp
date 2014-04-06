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
#include <primesieve/PrimeSieve.hpp>
#include <primesieve/ParallelPrimeSieve.hpp>
#include <primesieve/cancel_callback.hpp>
#include <primesieve/callback_t.hpp>

#include <stdint.h>
#include <stddef.h>
#include <exception>
#include <limits>
#include <cerrno>

//////////////////////////////////////////////////////////////////////
//                    Internal helper functions
//////////////////////////////////////////////////////////////////////

namespace {

const int BUFFER_BYTES = 128;

/// This is the C array's memory layout:
/// primes[index]   = first prime.
/// primes[index-1] = memory address of corresponding std::vector object.
/// primes[index-2] = integer type, e.g. INT_PRIMES.
///
template <typename T>
void* generate_primes_helper(uint64_t start, uint64_t stop, size_t* size, int type)
{
#if __cplusplus >= 201103L
  static_assert(BUFFER_BYTES % sizeof(T) == 0, "Prime type sizeof must be a power of 2.");
#endif
  std::vector<T>& primes = *(new std::vector<T>);
  try
  {
    size_t index = BUFFER_BYTES / sizeof(T);
    primes.resize(index, 0);
    reinterpret_cast<uintptr_t*>(&primes[index])[-1] = reinterpret_cast<uintptr_t>(&primes);
    reinterpret_cast<uintptr_t*>(&primes[index])[-2] = type;

    primesieve::generate_primes(start, stop, &primes);
    if (size)
      *size = primes.size() - index;
    return reinterpret_cast<void*>(&primes[index]);
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
/// primes[index]   = first prime.
/// primes[index-1] = memory address of corresponding std::vector object.
/// primes[index-2] = integer type, e.g. INT_PRIMES.
///
template <typename T>
void* generate_n_primes_helper(uint64_t n, uint64_t start, int type)
{
#if __cplusplus >= 201103L
  static_assert(BUFFER_BYTES % sizeof(T) == 0, "Prime type sizeof must be a power of 2.");
#endif
  std::vector<T>& primes = *(new std::vector<T>);
  try
  {
    size_t index = BUFFER_BYTES / sizeof(T);
    primes.resize(index, 0);
    reinterpret_cast<uintptr_t*>(&primes[index])[-1] = reinterpret_cast<uintptr_t>(&primes);
    reinterpret_cast<uintptr_t*>(&primes[index])[-2] = type;

    primesieve::generate_n_primes(n, start, &primes);

    return reinterpret_cast<void*>(&primes[index]);
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

void* primesieve_generate_primes(uint64_t start, uint64_t stop, size_t* size, int type)
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

void* primesieve_generate_n_primes(uint64_t n, uint64_t start, int type)
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

void primesieve_free(void* primes_c)
{
  if (primes_c)
  {
    uintptr_t pimpl = reinterpret_cast<uintptr_t*>(primes_c)[-1];
    uintptr_t type  = reinterpret_cast<uintptr_t*>(primes_c)[-2];
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

uint64_t primesieve_nth_prime(int64_t n, uint64_t start)
{
  try
  {
    primesieve::PrimeSieve ps;
    ps.setSieveSize(primesieve::get_sieve_size());
    return ps.nthPrime(n, start);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
  return PRIMESIEVE_ERROR;
}

uint64_t primesieve_parallel_nth_prime(int64_t n, uint64_t start)
{
  try
  {
    primesieve::ParallelPrimeSieve pps;
    pps.setSieveSize (primesieve::get_sieve_size());
    pps.setNumThreads(primesieve::get_num_threads());
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

uint64_t primesieve_count_primes(uint64_t start, uint64_t stop)
{
  try
  {
    primesieve::PrimeSieve ps;
    ps.setSieveSize(primesieve::get_sieve_size());
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
    primesieve::PrimeSieve ps;
    ps.setSieveSize(primesieve::get_sieve_size());
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
    primesieve::PrimeSieve ps;
    ps.setSieveSize(primesieve::get_sieve_size());
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
    primesieve::PrimeSieve ps;
    ps.setSieveSize(primesieve::get_sieve_size());
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
    primesieve::PrimeSieve ps;
    ps.setSieveSize(primesieve::get_sieve_size());
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
    primesieve::PrimeSieve ps;
    ps.setSieveSize(primesieve::get_sieve_size());
    return ps.countSextuplets(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
  return PRIMESIEVE_ERROR;
}

uint64_t primesieve_count_septuplets(uint64_t start, uint64_t stop)
{
  try
  {
    primesieve::PrimeSieve ps;
    ps.setSieveSize(primesieve::get_sieve_size());
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

uint64_t primesieve_parallel_count_primes(uint64_t start, uint64_t stop)
{
  try
  {
    primesieve::ParallelPrimeSieve pps;
    pps.setSieveSize (primesieve::get_sieve_size());
    pps.setNumThreads(primesieve::get_num_threads());
    return pps.countPrimes(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
  return PRIMESIEVE_ERROR;
}

uint64_t primesieve_parallel_count_twins(uint64_t start, uint64_t stop)
{
  try
  {
    primesieve::ParallelPrimeSieve pps;
    pps.setSieveSize (primesieve::get_sieve_size());
    pps.setNumThreads(primesieve::get_num_threads());
    return pps.countTwins(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
  return PRIMESIEVE_ERROR;
}

uint64_t primesieve_parallel_count_triplets(uint64_t start, uint64_t stop)
{
  try
  {
    primesieve::ParallelPrimeSieve pps;
    pps.setSieveSize (primesieve::get_sieve_size());
    pps.setNumThreads(primesieve::get_num_threads());
    return pps.countTriplets(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
  return PRIMESIEVE_ERROR;
}

uint64_t primesieve_parallel_count_quadruplets(uint64_t start, uint64_t stop)
{
  try
  {
    primesieve::ParallelPrimeSieve pps;
    pps.setSieveSize (primesieve::get_sieve_size());
    pps.setNumThreads(primesieve::get_num_threads());
    return pps.countQuadruplets(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
  return PRIMESIEVE_ERROR;
}

uint64_t primesieve_parallel_count_quintuplets(uint64_t start, uint64_t stop)
{
  try
  {
    primesieve::ParallelPrimeSieve pps;
    pps.setSieveSize (primesieve::get_sieve_size());
    pps.setNumThreads(primesieve::get_num_threads());
    return pps.countQuintuplets(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
  return PRIMESIEVE_ERROR;
}

uint64_t primesieve_parallel_count_sextuplets(uint64_t start, uint64_t stop)
{
  try
  {
    primesieve::ParallelPrimeSieve pps;
    pps.setSieveSize (primesieve::get_sieve_size());
    pps.setNumThreads(primesieve::get_num_threads());
    return pps.countSextuplets(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
  return PRIMESIEVE_ERROR;
}

uint64_t primesieve_parallel_count_septuplets(uint64_t start, uint64_t stop)
{
  try
  {
    primesieve::ParallelPrimeSieve pps;
    pps.setSieveSize (primesieve::get_sieve_size());
    pps.setNumThreads(primesieve::get_num_threads());
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

void primesieve_print_primes(uint64_t start, uint64_t stop)
{
  try
  {
    primesieve::PrimeSieve ps;
    ps.setSieveSize(primesieve::get_sieve_size());
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
    primesieve::PrimeSieve ps;
    ps.setSieveSize(primesieve::get_sieve_size());
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
    primesieve::PrimeSieve ps;
    ps.setSieveSize(primesieve::get_sieve_size());
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
    primesieve::PrimeSieve ps;
    ps.setSieveSize(primesieve::get_sieve_size());
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
    primesieve::PrimeSieve ps;
    ps.setSieveSize(primesieve::get_sieve_size());
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
    primesieve::PrimeSieve ps;
    ps.setSieveSize(primesieve::get_sieve_size());
    ps.printSextuplets(start, stop);
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
}

void primesieve_print_septuplets(uint64_t start, uint64_t stop)
{
  try
  {
    primesieve::PrimeSieve ps;
    ps.setSieveSize(primesieve::get_sieve_size());
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

void primesieve_callback_primes(uint64_t start, uint64_t stop, void (*callback)(uint64_t))
{
  try
  {
    primesieve::PrimeSieve ps;
    ps.setSieveSize(primesieve::get_sieve_size());
    // temporarily cast away extern "C" linkage
    ps.callbackPrimes_c(start, stop, reinterpret_cast<callback_t>(callback));
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
}

void primesieve_parallel_callback_primes(uint64_t start, uint64_t stop, void (*callback)(uint64_t, int))
{
  try
  {
    primesieve::ParallelPrimeSieve pps;
    pps.setSieveSize (primesieve::get_sieve_size());
    pps.setNumThreads(primesieve::get_num_threads());
    // temporarily cast away extern "C" linkage
    pps.callbackPrimes_c(start, stop, reinterpret_cast<callback_tn_t>(callback));
  }
  catch (std::exception&)
  {
    errno = EDOM;
  }
}

//////////////////////////////////////////////////////////////////////
//                        Getters and Setters
//////////////////////////////////////////////////////////////////////

int primesieve_get_sieve_size()
{
  return primesieve::get_sieve_size();
}

int primesieve_get_num_threads()
{
  return primesieve::get_num_threads();
}

uint64_t primesieve_get_max_stop()
{
  return primesieve::get_max_stop();
}

void primesieve_set_sieve_size(int sieve_size)
{
  primesieve::set_sieve_size(sieve_size);
}

void primesieve_set_num_threads(int num_threads)
{
  primesieve::set_num_threads(num_threads);
}

//////////////////////////////////////////////////////////////////////
//                           Miscellaneous
//////////////////////////////////////////////////////////////////////

int primesieve_test()
{
  return (primesieve::test() == true) ? 1 : 0;
}

} // extern "C"
