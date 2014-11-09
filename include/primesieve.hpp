///
/// @file   primesieve.hpp
/// @brief  primesieve C++ API. primesieve is a library for fast prime
///         number generation. In case an error occurs the functions
///         declared in this header will throw a
///         primesieve::primesieve_error exception (derived form
///         std::runtime_error).
///
/// Copyright (C) 2014 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRIMESIEVE_HPP
#define PRIMESIEVE_HPP

#define PRIMESIEVE_VERSION "5.4.1"
#define PRIMESIEVE_VERSION_MAJOR 5
#define PRIMESIEVE_VERSION_MINOR 4
#define PRIMESIEVE_VERSION_PATCH 1

#include "primesieve/PrimeSieve.hpp"
#include "primesieve/ParallelPrimeSieve.hpp"
#include "primesieve/Callback.hpp"
#include "primesieve/cancel_callback.hpp"
#include "primesieve/iterator.hpp"
#include "primesieve/PushBackPrimes.hpp"
#include "primesieve/primesieve_error.hpp"

#include <stdint.h>
#include <vector>

/// All of primesieve's C++ functions and classes are declared
/// inside this namespace.
///
namespace primesieve
{
  enum {
    /// Use all CPU cores for prime sieving.
    MAX_THREADS = -1
  };

  /// Store the primes <= stop in the primes vector.
  /// @pre stop <= 2^64 - 2^32 * 10.
  ///
  template <typename T>
  inline void generate_primes(uint64_t stop, std::vector<T>* primes)
  {
    if (primes)
    {
      PushBackPrimes<T> pb(*primes);
      pb.pushBackPrimes(0, stop);
    }
  }

  /// Store the primes within the interval [start, stop]
  /// in the primes vector.
  /// @pre stop <= 2^64 - 2^32 * 10.
  ///
  template <typename T>
  inline void generate_primes(uint64_t start, uint64_t stop, std::vector<T>* primes)
  {
    if (primes)
    {
      PushBackPrimes<T> pb(*primes);
      pb.pushBackPrimes(start, stop);
    }
  }

  /// Store the first n primes in the primes vector.
  template <typename T>
  inline void generate_n_primes(uint64_t n, std::vector<T>* primes)
  {
    if (primes)
    {
      PushBack_N_Primes<T> pb(*primes);
      pb.pushBack_N_Primes(n, 0);
    }
  }

  /// Store the first n primes >= start in the primes vector.
  /// @pre start <= 2^64 - 2^32 * 10.
  ///
  template <typename T>
  inline void generate_n_primes(uint64_t n, uint64_t start, std::vector<T>* primes)
  {
    if (primes)
    {
      PushBack_N_Primes<T> pb(*primes);
      pb.pushBack_N_Primes(n, start);
    }
  }

  /// Find the nth prime.
  /// @param n  if n = 0 finds the 1st prime >= start, <br/>
  ///           if n > 0 finds the nth prime > start, <br/>
  ///           if n < 0 finds the nth prime < start (backwards).
  /// @pre   start <= 2^64 - 2^32 * 11.
  ///
  uint64_t nth_prime(int64_t n, uint64_t start = 0);

  /// Find the nth prime in parallel.
  /// By default all CPU cores are used, use
  /// primesieve::set_num_threads(int) to change the number of
  /// threads.
  /// @param n  if n = 0 finds the 1st prime >= start, <br/>
  ///           if n > 0 finds the nth prime > start, <br/>
  ///           if n < 0 finds the nth prime < start (backwards).
  /// @pre   start <= 2^64 - 2^32 * 11.
  ///
  uint64_t parallel_nth_prime(int64_t n, uint64_t start = 0);

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

  /// Count the primes within the interval [start, stop] in
  /// parallel. By default all CPU cores are used, use
  /// primesieve::set_num_threads(int) to change the number of
  /// threads.
  /// @pre stop <= 2^64 - 2^32 * 10.
  ///
  uint64_t parallel_count_primes(uint64_t start, uint64_t stop);

  /// Count the twin primes within the interval [start, stop] in
  /// parallel. By default all CPU cores are used, use
  /// primesieve::set_num_threads(int) to change the number of
  /// threads.
  /// @pre stop <= 2^64 - 2^32 * 10.
  ///
  uint64_t parallel_count_twins(uint64_t start, uint64_t stop);

  /// Count the prime triplets within the interval [start, stop] in
  /// parallel. By default all CPU cores are used, use
  /// primesieve::set_num_threads(int) to change the number of
  /// threads.
  /// @pre stop <= 2^64 - 2^32 * 10.
  ///
  uint64_t parallel_count_triplets(uint64_t start, uint64_t stop);

  /// Count the prime quadruplets within the interval [start, stop] in
  /// parallel. By default all CPU cores are used, use
  /// primesieve::set_num_threads(int) to change the number of
  /// threads.
  /// @pre stop <= 2^64 - 2^32 * 10.
  ///
  uint64_t parallel_count_quadruplets(uint64_t start, uint64_t stop);

  /// Count the prime quintuplets within the interval [start, stop] in
  /// parallel. By default all CPU cores are used, use
  /// primesieve::set_num_threads(int) to change the number of
  /// threads.
  /// @pre stop <= 2^64 - 2^32 * 10.
  ///
  uint64_t parallel_count_quintuplets(uint64_t start, uint64_t stop);

  /// Count the prime sextuplets within the interval [start, stop] in
  /// parallel. By default all CPU cores are used, use
  /// primesieve::set_num_threads(int) to change the number of
  /// threads.
  /// @pre stop <= 2^64 - 2^32 * 10.
  ///
  uint64_t parallel_count_sextuplets(uint64_t start, uint64_t stop);

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

  /// Call back the primes within the interval [start, stop].
  /// @param callback  A callback function.
  /// @pre   stop <= 2^64 - 2^32 * 10.
  ///
  void callback_primes(uint64_t start, uint64_t stop, void (*callback)(uint64_t prime));

  /// Call back the primes within the interval [start, stop].
  /// @param callback  An object derived from primesieve::Callback<uint64_t>.
  /// @pre   stop <= 2^64 - 2^32 * 10.
  ///
  void callback_primes(uint64_t start, uint64_t stop, primesieve::Callback<uint64_t>* callback);

  /// Call back the primes within the interval [start, stop].
  /// This function is synchronized, only one thread at a time calls
  /// back primes. By default all CPU cores are used, use
  /// primesieve::set_num_threads(int) to change the number of
  /// threads.
  /// @warning         Primes are not called back in arithmetic order.
  /// @param callback  A callback function.
  /// @pre   stop      <= 2^64 - 2^32 * 10.
  ///
  void parallel_callback_primes(uint64_t start, uint64_t stop, void (*callback)(uint64_t prime));

  /// Call back the primes within the interval [start, stop].
  /// This function is synchronized, only one thread at a time calls
  /// back primes. By default all CPU cores are used, use
  /// primesieve::set_num_threads(int) to change the number of
  /// threads.
  /// @warning         Primes are not called back in arithmetic order.
  /// @param callback  An object derived from primesieve::Callback<uint64_t>.
  /// @pre   stop      <= 2^64 - 2^32 * 10.
  ///
  void parallel_callback_primes(uint64_t start, uint64_t stop, primesieve::Callback<uint64_t>* callback);

  /// Call back the primes within the interval [start, stop].
  /// This function is not synchronized, multiple threads call back
  /// primes in parallel. By default all CPU cores are used, use
  /// primesieve::set_num_threads(int) to change the number of
  /// threads.
  /// @warning         Primes are not called back in arithmetic order.
  /// @param callback  A callback function.
  /// @pre   stop      <= 2^64 - 2^32 * 10.
  ///
  void parallel_callback_primes(uint64_t start, uint64_t stop, void (*callback)(uint64_t prime, int thread_id));

  /// Call back the primes within the interval [start, stop].
  /// This function is not synchronized, multiple threads call back
  /// primes in parallel. By default all CPU cores are used, use
  /// primesieve::set_num_threads(int) to change the number of
  /// threads.
  /// @warning         Primes are not called back in arithmetic order.
  /// @param callback  An object derived from primesieve::Callback<uint64_t, int>.
  /// @pre   stop      <= 2^64 - 2^32 * 10.
  ///
  void parallel_callback_primes(uint64_t start, uint64_t stop, primesieve::Callback<uint64_t, int>* callback);

  /// Get the current set sieve size in kilobytes.
  int get_sieve_size();

  /// Get the current set number of threads.
  /// @note By default MAX_THREADS (-1) is returned.
  ///
  int get_num_threads();

  /// Returns the largest valid stop number for primesieve.
  /// @return (2^64-1) - (2^32-1) * 10.
  ///
  uint64_t get_max_stop();

  /// Set the sieve size in kilobytes.
  /// The best sieving performance is achieved with a sieve size of
  /// your CPU's L1 data cache size (per core). For sieving >= 10^17 a
  /// sieve size of your CPU's L2 cache size sometimes performs
  /// better.
  /// @param sieve_size Sieve size in kilobytes.
  /// @pre   sieve_size >= 1 && sieve_size <= 2048.
  ///
  void set_sieve_size(int sieve_size);

  /// Set the number of threads for use in subsequent
  /// primesieve::parallel_* function calls. Note that this only
  /// changes the number of threads for the current process.
  /// @param num_threads  Number of threads for sieving
  ///                     or MAX_THREADS to use all CPU cores.
  ///
  void set_num_threads(int num_threads);

  /// Run extensive correctness tests.
  /// The tests last about one minute on a quad core CPU from
  /// 2013 and use up to 1 gigabyte of memory.
  /// @return true if success else false.
  ///
  bool test();
}

#endif
