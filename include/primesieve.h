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

#include <primesieve/soe/PrimeSieve.h>
#include <primesieve/soe/ParallelPrimeSieve.h>
#include <primesieve/soe/primesieve_error.h>
#include <primesieve/soe/PrimeSieveCallback.h>
#include <primesieve/soe/stop_primesieve.h>
#include <algorithm>
#include <stdint.h>

namespace primesieve
{
  enum {
    /// Use all CPU cores for prime sieving.
    MAX_THREADS = -1
  };

  /// Callback the primes within the interval [start, stop].
  /// @param callback  A callback function.
  ///
  void callback_primes(uint32_t start, uint32_t stop, void (*callback)(uint32_t prime));

  /// Callback the primes within the interval [start, stop].
  /// @param callback  A callback function.
  ///
  void callback_primes(uint64_t start, uint64_t stop, void (*callback)(uint64_t prime));

  /// Callback the primes within the interval [start, stop].
  /// @param callback  An object derived from PrimeSieveCallback<uint32_t>.
  ///
  void callback_primes(uint32_t start, uint32_t stop, PrimeSieveCallback<uint32_t>* callback);

  /// Callback the primes within the interval [start, stop].
  /// @param callback  An object derived from PrimeSieveCallback<uint64_t>.
  ///
  void callback_primes(uint64_t start, uint64_t stop, PrimeSieveCallback<uint64_t>* callback);

  /// Callback the primes within the interval [start, stop].
  /// @param threads   Number of threads.
  /// @param callback  A callback function.
  ///
  void callback_primes(uint32_t start, uint32_t stop, int threads, void (*callback)(uint32_t prime));

  /// Callback the primes within the interval [start, stop].
  /// @param threads   Number of threads.
  /// @param callback  A callback function.
  ///
  void callback_primes(uint64_t start, uint64_t stop, int threads, void (*callback)(uint64_t prime));

  /// Callback the primes within the interval [start, stop].
  /// @param threads   Number of threads.
  /// @param callback  A callback function.
  ///
  void callback_primes(uint64_t start, uint64_t stop, int threads, void (*callback)(uint64_t prime, int thread_id));

  /// Callback the primes within the interval [start, stop].
  /// @param threads   Number of threads.
  /// @param callback  An object derived from PrimeSieveCallback<uint32_t>.
  ///
  void callback_primes(uint32_t start, uint32_t stop, int threads, PrimeSieveCallback<uint32_t>* callback);

  /// Callback the primes within the interval [start, stop].
  /// @param threads   Number of threads.
  /// @param callback  An object derived from PrimeSieveCallback<uint64_t>.
  ///
  void callback_primes(uint64_t start, uint64_t stop, int threads, PrimeSieveCallback<uint64_t>* callback);

  /// Callback the primes within the interval [start, stop].
  /// @param threads   Number of threads.
  /// @param callback  An object derived from PrimeSieveCallback<uint64_t, int>.
  ///
  void callback_primes(uint64_t start, uint64_t stop, int threads, PrimeSieveCallback<uint64_t, int>* callback);

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

  /// Count the primes within the interval [start, stop].
  /// @param threads  Number of threads.
  ///
  uint64_t count_primes(uint64_t start, uint64_t stop, int threads);

  /// Count the twin primes within the interval [start, stop].
  /// @param threads  Number of threads.
  ///
  uint64_t count_twins(uint64_t start, uint64_t stop, int threads);

  /// Count the prime  within the interval [start, stop].
  /// @param threads  Number of threads.
  ///
  uint64_t count_triplets(uint64_t start, uint64_t stop, int threads);

  /// Count the prime  within the interval [start, stop].
  /// @param threads  Number of threads.
  ///
  uint64_t count_quadruplets(uint64_t start, uint64_t stop, int threads);

  /// Count the prime  within the interval [start, stop].
  /// @param threads  Number of threads.
  ///
  uint64_t count_quintuplets(uint64_t start, uint64_t stop, int threads);

  /// Count the prime  within the interval [start, stop].
  /// @param threads  Number of threads.
  ///
  uint64_t count_sextuplets(uint64_t start, uint64_t stop, int threads);

  /// Count the prime  within the interval [start, stop].
  /// @param threads  Number of threads.
  ///
  uint64_t count_septuplets(uint64_t start, uint64_t stop, int threads);

  /// Generate the primes within the interval [start, stop]
  /// and store them in the primes vector.
  ///
  template <typename T>
  inline void generate_primes(uint64_t start, uint64_t stop, std::vector<T>* primes)
  {
    PrimeSieve ps;
    ps.generatePrimes(start, stop, primes);
  }

  /// Generate the primes within the interval [start, stop]
  /// and store them in the primes vector. The primes vector will
  /// be sorted at the end to restore arithmetic order
  /// (multi-threading generates primes in random order).
  ///
  template <typename T>
  inline void generate_primes(uint64_t start, uint64_t stop, int threads, std::vector<T>* primes)
  {
    if (primes)
    {
      ParallelPrimeSieve pps;
      pps.setNumThreads(threads);
      pps.generatePrimes(start, stop, primes);
      std::sort(primes->begin(), primes->end());
    }
  }

  /// Generate the first n primes and store them in the primes vector.
  template <typename T>
  inline void generate_n_primes(uint64_t n, std::vector<T>* primes)
  {
    PrimeSieve ps;
    ps.generate_N_Primes(n, primes);
  }

  /// Generate the first n primes and store them in the primes
  /// vector. The primes vector will be sorted at the end to
  /// restore arithmetic order (multi-threading generates primes
  /// in random order).
  ///
  template <typename T>
  inline void generate_n_primes(uint64_t n, int threads, std::vector<T>* primes)
  {
    if (primes)
    {
      ParallelPrimeSieve pps;
      pps.setNumThreads(threads);
      pps.generate_N_Primes(n, primes);
      std::sort(primes->begin(), primes->end());
    }
  }

  /// Generate the first n primes larger than or equal to start
  /// and store them in the primes vector. The primes vector will be
  /// sorted at the end to restore arithmetic order
  /// (multi-threading generates primes in random order).
  ///
  template <typename T>
  inline void generate_n_primes(uint64_t n, uint64_t start, int threads, std::vector<T>* primes)
  {
    if (primes)
    {
      ParallelPrimeSieve pps;
      pps.setNumThreads(threads);
      pps.generate_N_Primes(n, start, primes);
      std::sort(primes->begin(), primes->end());
    }
  }

  /// Find the nth prime.
  uint64_t nth_prime(uint64_t n);

  /// Find the nth prime.
  /// @param threads  Number of threads.
  ///
  uint64_t nth_prime(uint64_t n, int threads);

  /// Find the nth prime larger than or equal to start.
  /// @param threads  Number of threads.
  ///
  uint64_t nth_prime(uint64_t n, uint64_t start, int threads);

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

  /// Print the primes within the interval [start, stop]
  /// to the standard output.
  /// @warning Primes are printed in random order if threads > 1.
  ///
  void print_primes(uint64_t start, uint64_t stop, int threads);

  /// Print the twin primes within the interval [start, stop]
  /// to the standard output.
  /// @warning Twin primes are printed in random order if threads > 1.
  ///
  void print_twins(uint64_t start, uint64_t stop, int threads);

  /// Print the prime triplets within the interval [start, stop]
  /// to the standard output.
  /// @warning Prime triplets are printed in random order if threads > 1.
  ///
  void print_triplets(uint64_t start, uint64_t stop, int threads);

  /// Print the prime quadruplets within the interval [start, stop]
  /// to the standard output.
  /// @warning Prime quadruplets are printed in random order if threads > 1.
  ///
  void print_quadruplets(uint64_t start, uint64_t stop, int threads);

  /// Print the prime quintuplets within the interval [start, stop]
  /// to the standard output.
  /// @warning Prime quintuplets are printed in random order if threads > 1.
  ///
  void print_quintuplets(uint64_t start, uint64_t stop, int threads);

  /// Print the prime sextuplets within the interval [start, stop]
  /// to the standard output.
  /// @warning Prime sextuplets are printed in random order if threads > 1.
  ///
  void print_sextuplets(uint64_t start, uint64_t stop, int threads);

  /// Print the prime septuplets within the interval [start, stop]
  /// to the standard output.
  /// @warning Prime septuplets are printed in random order if threads > 1.
  ///
  void print_septuplets(uint64_t start, uint64_t stop, int threads);
}

#endif
