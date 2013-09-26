///
/// @file   primesieve.h
/// @brief  C++ API of the primesieve library.
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRIMESIEVE_H
#define PRIMESIEVE_H

#define PRIMESIEVE_VERSION "@PACKAGE_VERSION@"
#define PRIMESIEVE_VERSION_MAJOR @PACKAGE_VERSION_MAJOR@
#define PRIMESIEVE_VERSION_MINOR @PACKAGE_VERSION_MINOR@
#define PRIMESIEVE_YEAR "@PACKAGE_YEAR@"

#include "primesieve/soe/PrimeSieve.h"
#include "primesieve/soe/ParallelPrimeSieve.h"
#include "primesieve/soe/primesieve_error.h"
#include "primesieve/soe/PrimeSieveCallback.h"
#include "primesieve/soe/stop_primesieve.h"

#include <stdint.h>

namespace primesieve
{
  enum {
    /// Use all CPU cores for prime sieving.
    MAX_THREADS = -1
  };

  /// Call back the primes within the interval [start, stop].
  /// @param callback  A callback function.
  ///
  void callback_primes(uint32_t start, uint32_t stop, void (*callback)(uint32_t prime));

  /// Call back the primes within the interval [start, stop].
  /// @param callback  A callback function.
  ///
  void callback_primes(uint64_t start, uint64_t stop, void (*callback)(uint64_t prime));

  /// Call back the primes within the interval [start, stop].
  /// @param callback  An object derived from PrimeSieveCallback<uint32_t>.
  ///
  void callback_primes(uint32_t start, uint32_t stop, PrimeSieveCallback<uint32_t>* callback);

  /// Call back the primes within the interval [start, stop].
  /// @param callback  An object derived from PrimeSieveCallback<uint64_t>.
  ///
  void callback_primes(uint64_t start, uint64_t stop, PrimeSieveCallback<uint64_t>* callback);

  /// Count the primes within the interval [start, stop].
  uint64_t count_primes(uint64_t start, uint64_t stop);

  /// Count the twin primes within the interval [start, stop].
  uint64_t count_twins(uint64_t start, uint64_t stop);

  /// Count the prime triplets within the interval [start, stop].
  uint64_t count_triplets(uint64_t start, uint64_t stop);

  /// Count the prime quadruplets within the interval [start, stop].
  uint64_t count_quadruplets(uint64_t start, uint64_t stop);

  /// Count the prime quintuplets within the interval [start, stop].
  uint64_t count_quintuplets(uint64_t start, uint64_t stop);

  /// Count the prime sextuplets within the interval [start, stop].
  uint64_t count_sextuplets(uint64_t start, uint64_t stop);

  /// Count the prime septuplets within the interval [start, stop].
  uint64_t count_septuplets(uint64_t start, uint64_t stop);

  /// Generate the primes <= stop and store
  /// them in the primes vector.
  ///
  template <typename T>
  inline void generate_primes(uint64_t stop, std::vector<T>* primes)
  {
    PrimeSieve ps;
    ps.generatePrimes(0, stop, primes);
  }

  /// Generate the primes within the interval [start, stop]
  /// and store them in the primes vector.
  ///
  template <typename T>
  inline void generate_primes(uint64_t start, uint64_t stop, std::vector<T>* primes)
  {
    PrimeSieve ps;
    ps.generatePrimes(start, stop, primes);
  }

  /// Generate the first n primes and store them in the primes vector.
  template <typename T>
  inline void generate_n_primes(uint64_t n, std::vector<T>* primes)
  {
    PrimeSieve ps;
    ps.generate_N_Primes(n, primes);
  }

  /// Generate the first n primes >= start and store them
  /// in the primes vector.
  ///
  template <typename T>
  inline void generate_n_primes(uint64_t n, uint64_t start, std::vector<T>* primes)
  {
    PrimeSieve ps;
    ps.generate_N_Primes(n, start, primes);
  }

  /// Find the nth prime.
  /// @param start  Start nth prime search at this offset.
  ///
  uint64_t nth_prime(uint64_t n, uint64_t start = 0);

  /// Print the primes within the interval [start, stop]
  /// to the standard output.
  ///
  void print_primes(uint64_t start, uint64_t stop);

  /// Print the twin primes within the interval [start, stop]
  /// to the standard output.
  ///
  void print_twins(uint64_t start, uint64_t stop);

  /// Print the prime triplets within the interval [start, stop]
  /// to the standard output.
  ///
  void print_triplets(uint64_t start, uint64_t stop);

  /// Print the prime quadruplets within the interval [start, stop]
  /// to the standard output.
  ///
  void print_quadruplets(uint64_t start, uint64_t stop);

  /// Print the prime quintuplets within the interval [start, stop]
  /// to the standard output.
  ///
  void print_quintuplets(uint64_t start, uint64_t stop);

  /// Print the prime sextuplets within the interval [start, stop]
  /// to the standard output.
  ///
  void print_sextuplets(uint64_t start, uint64_t stop);

  /// Print the prime septuplets within the interval [start, stop]
  /// to the standard output.
  ///
  void print_septuplets(uint64_t start, uint64_t stop);

  /// Call back the primes within the interval [start, stop].
  /// This function is synchronized, only one thread at a time calls
  /// back primes.
  /// @warning         Primes are not called back in arithmetic order.
  /// @param callback  A callback function.
  /// @param threads   Number of threads.
  ///
  void parallel_callback_primes(uint32_t start, uint32_t stop, void (*callback)(uint32_t prime), int threads = MAX_THREADS);

  /// Call back the primes within the interval [start, stop].
  /// This function is synchronized, only one thread at a time calls
  /// back primes.
  /// @warning         Primes are not called back in arithmetic order.
  /// @param callback  A callback function.
  /// @param threads   Number of threads.
  ///
  void parallel_callback_primes(uint64_t start, uint64_t stop, void (*callback)(uint64_t prime), int threads = MAX_THREADS);

  /// Call back the primes within the interval [start, stop].
  /// This function is not synchronized, multiple threads call back
  /// primes in parallel.
  /// @warning         Primes are not called back in arithmetic order.
  /// @param callback  A callback function.
  /// @param threads   Number of threads.
  ///
  void parallel_callback_primes(uint64_t start, uint64_t stop, void (*callback)(uint64_t prime, int thread_id), int threads = MAX_THREADS);

  /// Call back the primes within the interval [start, stop].
  /// This function is synchronized, only one thread at a time calls
  /// back primes.
  /// @warning         Primes are not called back in arithmetic order.
  /// @param callback  An object derived from PrimeSieveCallback<uint32_t>.
  /// @param threads   Number of threads.
  ///
  void parallel_callback_primes(uint32_t start, uint32_t stop, PrimeSieveCallback<uint32_t>* callback, int threads = MAX_THREADS);

  /// Call back the primes within the interval [start, stop].
  /// This function is synchronized, only one thread at a time calls
  /// back primes.
  /// @warning         Primes are not called back in arithmetic order.
  /// @param callback  An object derived from PrimeSieveCallback<uint64_t>.
  /// @param threads   Number of threads.
  ///
  void parallel_callback_primes(uint64_t start, uint64_t stop, PrimeSieveCallback<uint64_t>* callback, int threads = MAX_THREADS);

  /// Call back the primes within the interval [start, stop].
  /// This function is not synchronized, multiple threads call back
  /// primes in parallel.
  /// @warning         Primes are not called back in arithmetic order.
  /// @param callback  An object derived from PrimeSieveCallback<uint64_t, int>.
  /// @param threads   Number of threads.
  ///
  void parallel_callback_primes(uint64_t start, uint64_t stop, PrimeSieveCallback<uint64_t, int>* callback, int threads = MAX_THREADS);

  /// Count the primes within the interval [start, stop].
  /// @param threads  Number of threads.
  ///
  uint64_t parallel_count_primes(uint64_t start, uint64_t stop, int threads = MAX_THREADS);

  /// Count the twin primes within the interval [start, stop].
  /// @param threads  Number of threads.
  ///
  uint64_t parallel_count_twins(uint64_t start, uint64_t stop, int threads = MAX_THREADS);

  /// Count the prime triplets within the interval [start, stop].
  /// @param threads  Number of threads.
  ///
  uint64_t parallel_count_triplets(uint64_t start, uint64_t stop, int threads = MAX_THREADS);

  /// Count the prime quadruplets within the interval [start, stop].
  /// @param threads  Number of threads.
  ///
  uint64_t parallel_count_quadruplets(uint64_t start, uint64_t stop, int threads = MAX_THREADS);

  /// Count the prime quintuplets within the interval [start, stop].
  /// @param threads  Number of threads.
  ///
  uint64_t parallel_count_quintuplets(uint64_t start, uint64_t stop, int threads = MAX_THREADS);

  /// Count the prime sextuplets within the interval [start, stop].
  /// @param threads  Number of threads.
  ///
  uint64_t parallel_count_sextuplets(uint64_t start, uint64_t stop, int threads = MAX_THREADS);

  /// Count the prime septuplets within the interval [start, stop].
  /// @param threads  Number of threads.
  ///
  uint64_t parallel_count_septuplets(uint64_t start, uint64_t stop, int threads = MAX_THREADS);

  /// Find the nth prime.
  /// @param start    Start nth prime search at this offset.
  /// @param threads  Number of threads.
  ///
  uint64_t parallel_nth_prime(uint64_t n, uint64_t start = 0, int threads = MAX_THREADS);
}

#endif
