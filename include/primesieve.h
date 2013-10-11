///
/// @file   primesieve.h
/// @brief  C++ API of primesieve. primesieve is a fast library for
///         prime number generation. This header contains all of
///         primesieve's function declarations and must be included in
///         your C++ source code in order to use primesieve.
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRIMESIEVE_H
#define PRIMESIEVE_H

#define PRIMESIEVE_VERSION "5.0.0"
#define PRIMESIEVE_VERSION_MAJOR 5
#define PRIMESIEVE_VERSION_MINOR 0
#define PRIMESIEVE_VERSION_PATCH 0
#define PRIMESIEVE_YEAR "2013"

#ifdef __cplusplus
#include "primesieve/soe/PrimeSieve.h"
#include "primesieve/soe/ParallelPrimeSieve.h"
#include "primesieve/soe/primesieve_error.h"
#include "primesieve/soe/prime_iterator.h"
#include "primesieve/soe/PrimeSieveCallback.h"
#include "primesieve/soe/PushBackPrimes.h"
#include "primesieve/soe/stop_primesieve.h"

#include <vector>
#include <string>
#endif

#include <stdint.h>

#ifdef __cplusplus
/// All of primesieve's functions and classes are declared
/// inside this namespace.
///
namespace primesieve
{
  /// Declare the public pure-C API.
  /// The primesieve namespace is preserved for C++ applications.
  extern "C"
  {
#endif
    enum {
      /// Use all CPU cores for prime sieving.
      MAX_THREADS
    };

    /// Call back the primes within the interval [start, stop].
    /// @param callback  A callback function.
    /// @pre   stop <= 2^64 - 2^32 * 10.
    ///
    void callback_primes(uint64_t start, uint64_t stop, void (*callback)(uint64_t prime));

    /// Call back the primes within the interval [start, stop].
    /// This function is not synchronized, multiple threads call back
    /// primes in parallel.
    /// @warning         Primes are not called back in arithmetic order.
    /// @param callback  A callback function.
    /// @param threads   Number of threads.
    /// @pre   stop      <= 2^64 - 2^32 * 10.
    ///
    void parallel_callback_primes(uint64_t start, uint64_t stop, void (*callback)(uint64_t prime, int thread_id), int threads);

    /// Count the primes within the interval [start, stop].
    /// @pre stop <= 2^64 - 2^32 * 10.
    ///
    uint64_t count_primes(uint64_t start, uint64_t stop);

    /// Count the twin primes within the interval [start, stop].
    /// @pre stop <= 2^64 - 2^32 * 10.
    ///
    uint64_t count_twins(uint64_t start, uint64_t stop);

    /// Count the prime triplets within the interval [start, stop].
    /// @pre stop <= 2^64 - 2^32 * 10.
    ///
    uint64_t count_triplets(uint64_t start, uint64_t stop);

    /// Count the prime quadruplets within the interval [start, stop].
    /// @pre stop <= 2^64 - 2^32 * 10.
    ///
    uint64_t count_quadruplets(uint64_t start, uint64_t stop);

    /// Count the prime quintuplets within the interval [start, stop].
    /// @pre stop <= 2^64 - 2^32 * 10.
    ///
    uint64_t count_quintuplets(uint64_t start, uint64_t stop);

    /// Count the prime sextuplets within the interval [start, stop].
    /// @pre stop <= 2^64 - 2^32 * 10.
    ///
    uint64_t count_sextuplets(uint64_t start, uint64_t stop);

    /// Count the prime septuplets within the interval [start, stop].
    /// @pre stop <= 2^64 - 2^32 * 10.
    ///
    uint64_t count_septuplets(uint64_t start, uint64_t stop);

    /// Returns the largest valid stop number for primesieve.
    /// @return 2^64 - 2^32 * 10.
    ///
    uint64_t max_stop();

    /// Find the nth prime.
    /// @param start  Start nth prime search at this offset.
    /// @pre   start  <= 2^64 - 2^32 * 10.
    ///
    uint64_t nth_prime(uint64_t n, uint64_t start);

    /// Find the nth prime.
    /// @param start    Start nth prime search at this offset.
    /// @param threads  Number of threads.
    /// @pre   start    <= 2^64 - 2^32 * 10.
    ///
    uint64_t parallel_nth_prime(uint64_t n, uint64_t start, int threads);

    /// Count the primes within the interval [start, stop].
    /// @param threads  Number of threads.
    /// @pre   stop     <= 2^64 - 2^32 * 10.
    ///
    uint64_t parallel_count_primes(uint64_t start, uint64_t stop, int threads);

    /// Count the twin primes within the interval [start, stop].
    /// @param threads  Number of threads.
    /// @pre   stop     <= 2^64 - 2^32 * 10.
    ///
    uint64_t parallel_count_twins(uint64_t start, uint64_t stop, int threads);

    /// Count the prime triplets within the interval [start, stop].
    /// @param threads  Number of threads.
    /// @pre   stop     <= 2^64 - 2^32 * 10.
    ///
    uint64_t parallel_count_triplets(uint64_t start, uint64_t stop, int threads);

    /// Count the prime quadruplets within the interval [start, stop].
    /// @param threads  Number of threads.
    /// @pre   stop     <= 2^64 - 2^32 * 10.
    ///
    uint64_t parallel_count_quadruplets(uint64_t start, uint64_t stop, int threads);

    /// Count the prime quintuplets within the interval [start, stop].
    /// @param threads  Number of threads.
    /// @pre   stop     <= 2^64 - 2^32 * 10.
    ///
    uint64_t parallel_count_quintuplets(uint64_t start, uint64_t stop, int threads);

    /// Count the prime sextuplets within the interval [start, stop].
    /// @param threads  Number of threads.
    /// @pre   stop     <= 2^64 - 2^32 * 10.
    ///
    uint64_t parallel_count_sextuplets(uint64_t start, uint64_t stop, int threads);

    /// Count the prime septuplets within the interval [start, stop].
    /// @param threads  Number of threads.
    /// @pre   stop     <= 2^64 - 2^32 * 10.
    ///
    uint64_t parallel_count_septuplets(uint64_t start, uint64_t stop, int threads);

    /// Print the primes within the interval [start, stop]
    /// to the standard output.
    /// @pre stop <= 2^64 - 2^32 * 10.
    ///
    void print_primes(uint64_t start, uint64_t stop);

    /// Print the twin primes within the interval [start, stop]
    /// to the standard output.
    /// @pre stop <= 2^64 - 2^32 * 10.
    ///
    void print_twins(uint64_t start, uint64_t stop);

    /// Print the prime triplets within the interval [start, stop]
    /// to the standard output.
    /// @pre stop <= 2^64 - 2^32 * 10.
    ///
    void print_triplets(uint64_t start, uint64_t stop);

    /// Print the prime quadruplets within the interval [start, stop]
    /// to the standard output.
    /// @pre stop <= 2^64 - 2^32 * 10.
    ///
    void print_quadruplets(uint64_t start, uint64_t stop);

    /// Print the prime quintuplets within the interval [start, stop]
    /// to the standard output.
    /// @pre stop <= 2^64 - 2^32 * 10.
    ///
    void print_quintuplets(uint64_t start, uint64_t stop);

    /// Print the prime sextuplets within the interval [start, stop]
    /// to the standard output.
    /// @pre stop <= 2^64 - 2^32 * 10.
    ///
    void print_sextuplets(uint64_t start, uint64_t stop);

    /// Print the prime septuplets within the interval [start, stop]
    /// to the standard output.
    /// @pre stop <= 2^64 - 2^32 * 10.
    ///
    void print_septuplets(uint64_t start, uint64_t stop);

    /// Run extensive correctness tests.
    /// The tests last about one minute on a quad core CPU from
    /// 2013 and use up to 1 gigabyte of memory.
    /// @return true  If no error occurred else false.
    ///
    int test();
#ifdef __cplusplus
  }
  /// The remaining functions are spcefic to the C++ API.

  /// Find the nth prime.
  /// @param start  Start nth prime search at this offset.
  /// @pre   start  <= 2^64 - 2^32 * 10.
  ///
  uint64_t nth_prime(uint64_t n);

  /// Find the nth prime.
  /// @param start    Start nth prime search at this offset.
  /// @pre   start    <= 2^64 - 2^32 * 10.
  ///
  uint64_t parallel_nth_prime(uint64_t n, uint64_t start = 0);

  /// Generate the primes <= stop and store
  /// them in the primes vector.
  /// @pre stop <= 2^64 - 2^32 * 10.
  ///
  template <typename T>
  inline void generate_primes(uint64_t stop, std::vector<T>* primes)
  {
    if (primes)
    {
      soe::PushBackPrimes<T> pb(*primes);
      pb.pushBackPrimes(0, stop);
    }
  }

  /// Generate the primes within the interval [start, stop]
  /// and store them in the primes vector.
  /// @pre stop <= 2^64 - 2^32 * 10.
  ///
  template <typename T>
  inline void generate_primes(uint64_t start, uint64_t stop, std::vector<T>* primes)
  {
    if (primes)
    {
      soe::PushBackPrimes<T> pb(*primes);
      pb.pushBackPrimes(start, stop);
    }
  }

  /// Generate the first n primes and store them in the primes vector.
  template <typename T>
  inline void generate_n_primes(uint64_t n, std::vector<T>* primes)
  {
    if (primes)
    {
      soe::PushBack_N_Primes<T> pb(*primes);
      pb.pushBack_N_Primes(n, 0);
    }
  }

  /// Generate the first n primes >= start and store them
  /// in the primes vector.
  /// @pre start <= 2^64 - 2^32 * 10.
  ///
  template <typename T>
  inline void generate_n_primes(uint64_t n, uint64_t start, std::vector<T>* primes)
  {
    if (primes)
    {
      soe::PushBack_N_Primes<T> pb(*primes);
      pb.pushBack_N_Primes(n, start);
    }
  }

  /// Returns the largest valid stop number for primesieve.
  /// @return "2^64 - 2^32 * 10".
  ///
  std::string max_stop_string();

  /// Call back the primes within the interval [start, stop].
  /// @param callback  An object derived from PrimeSieveCallback<uint64_t>.
  /// @pre   stop <= 2^64 - 2^32 * 10.
  ///
  void callback_primes(uint64_t start, uint64_t stop, PrimeSieveCallback<uint64_t>* callback);

  /// Call back the primes within the interval [start, stop].
  /// This function is synchronized, only one thread at a time calls
  /// back primes.
  /// @warning         Primes are not called back in arithmetic order.
  /// @param callback  A callback function.
  /// @param threads   Number of threads.
  /// @pre   stop      <= 2^64 - 2^32 * 10.
  ///
  void parallel_callback_primes(uint64_t start, uint64_t stop, void (*callback)(uint64_t prime), int threads = MAX_THREADS);

  /// Call back the primes within the interval [start, stop].
  /// This function is not synchronized, multiple threads call back
  /// primes in parallel.
  /// @warning         Primes are not called back in arithmetic order.
  /// @param callback  A callback function.
  /// @pre   stop      <= 2^64 - 2^32 * 10.
  ///
  void parallel_callback_primes(uint64_t start, uint64_t stop, void (*callback)(uint64_t prime, int thread_id));

  /// Call back the primes within the interval [start, stop].
  /// This function is synchronized, only one thread at a time calls
  /// back primes.
  /// @warning         Primes are not called back in arithmetic order.
  /// @param callback  An object derived from PrimeSieveCallback<uint64_t>.
  /// @param threads   Number of threads.
  /// @pre   stop      <= 2^64 - 2^32 * 10.
  ///
  void parallel_callback_primes(uint64_t start, uint64_t stop, PrimeSieveCallback<uint64_t>* callback, int threads = MAX_THREADS);

  /// Call back the primes within the interval [start, stop].
  /// This function is not synchronized, multiple threads call back
  /// primes in parallel.
  /// @warning         Primes are not called back in arithmetic order.
  /// @param callback  An object derived from PrimeSieveCallback<uint64_t, int>.
  /// @param threads   Number of threads.
  /// @pre   stop      <= 2^64 - 2^32 * 10.
  ///
  void parallel_callback_primes(uint64_t start, uint64_t stop, PrimeSieveCallback<uint64_t, int>* callback, int threads = MAX_THREADS);

  /// Count the primes within the interval [start, stop].
  /// @pre   stop     <= 2^64 - 2^32 * 10.
  ///
  uint64_t parallel_count_primes(uint64_t start, uint64_t stop);

  /// Count the twin primes within the interval [start, stop].
  /// @pre   stop     <= 2^64 - 2^32 * 10.
  ///
  uint64_t parallel_count_twins(uint64_t start, uint64_t stop);

  /// Count the prime triplets within the interval [start, stop].
  /// @pre   stop     <= 2^64 - 2^32 * 10.
  ///
  uint64_t parallel_count_triplets(uint64_t start, uint64_t stop);

  /// Count the prime quadruplets within the interval [start, stop].
  /// @pre   stop     <= 2^64 - 2^32 * 10.
  ///
  uint64_t parallel_count_quadruplets(uint64_t start, uint64_t stop);

  /// Count the prime quintuplets within the interval [start, stop].
  /// @pre   stop     <= 2^64 - 2^32 * 10.
  ///
  uint64_t parallel_count_quintuplets(uint64_t start, uint64_t stop);

  /// Count the prime sextuplets within the interval [start, stop].
  /// @pre   stop     <= 2^64 - 2^32 * 10.
  ///
  uint64_t parallel_count_sextuplets(uint64_t start, uint64_t stop);

  /// Count the prime septuplets within the interval [start, stop].
  /// @pre   stop     <= 2^64 - 2^32 * 10.
  ///
  uint64_t parallel_count_septuplets(uint64_t start, uint64_t stop);
}
#endif

#endif
