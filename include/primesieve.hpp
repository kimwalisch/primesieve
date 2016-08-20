///
/// @file   primesieve.hpp
/// @brief  primesieve C++ API. primesieve is a library for fast prime
///         number generation, in case an error occurs a
///         primesieve::primesieve_error exception (derived form
///         std::runtime_error) will be thrown.
///
/// Copyright (C) 2016 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License.
///

#ifndef PRIMESIEVE_HPP
#define PRIMESIEVE_HPP

#define PRIMESIEVE_VERSION "5.7.2"
#define PRIMESIEVE_VERSION_MAJOR 5
#define PRIMESIEVE_VERSION_MINOR 7
#define PRIMESIEVE_VERSION_PATCH 2

#include <primesieve/PrimeSieve.hpp>
#include <primesieve/ParallelPrimeSieve.hpp>
#include <primesieve/Callback.hpp>
#include <primesieve/cancel_callback.hpp>
#include <primesieve/iterator.hpp>
#include <primesieve/PushBackPrimes.hpp>
#include <primesieve/primesieve_error.hpp>

#include <stdint.h>
#include <vector>
#include <string>

/// Contains primesieve's C++ functions and classes.
namespace primesieve {

/// Store the primes <= stop in the primes vector.
template <typename T>
inline void generate_primes(uint64_t stop, std::vector<T>* primes)
{
  if (primes)
  {
    PushBackPrimes<std::vector<T> > pb(*primes);
    pb.pushBackPrimes(0, stop);
  }
}

/// Store the primes within the interval [start, stop]
/// in the primes vector.
///
template <typename T>
inline void generate_primes(uint64_t start, uint64_t stop, std::vector<T>* primes)
{
  if (primes)
  {
    PushBackPrimes<std::vector<T> > pb(*primes);
    pb.pushBackPrimes(start, stop);
  }
}

/// Store the first n primes in the primes vector.
template <typename T>
inline void generate_n_primes(uint64_t n, std::vector<T>* primes)
{
  if (primes)
  {
    PushBack_N_Primes<std::vector<T> > pb(*primes);
    pb.pushBack_N_Primes(n, 0);
  }
}

/// Store the first n primes >= start in the primes vector.
template <typename T>
inline void generate_n_primes(uint64_t n, uint64_t start, std::vector<T>* primes)
{
  if (primes)
  {
    PushBack_N_Primes<std::vector<T> > pb(*primes);
    pb.pushBack_N_Primes(n, start);
  }
}

/// Find the nth prime.
/// @param n  if n = 0 finds the 1st prime >= start, <br/>
///           if n > 0 finds the nth prime > start, <br/>
///           if n < 0 finds the nth prime < start (backwards).
///
uint64_t nth_prime(int64_t n, uint64_t start = 0);

/// Find the nth prime in parallel.
/// By default all CPU cores are used, use
/// primesieve::set_num_threads(int) to change the number of
/// threads.
/// @param n  if n = 0 finds the 1st prime >= start, <br/>
///           if n > 0 finds the nth prime > start, <br/>
///           if n < 0 finds the nth prime < start (backwards).
///
uint64_t parallel_nth_prime(int64_t n, uint64_t start = 0);

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

/// Count the primes within the interval [start, stop] in
/// parallel. By default all CPU cores are used, use
/// primesieve::set_num_threads(int) to change the number of
/// threads.
///
uint64_t parallel_count_primes(uint64_t start, uint64_t stop);

/// Count the twin primes within the interval [start, stop]
/// in parallel. By default all CPU cores are used, use
/// primesieve::set_num_threads(int) to change the number of
/// threads.
///
uint64_t parallel_count_twins(uint64_t start, uint64_t stop);

/// Count the prime triplets within the interval [start, stop]
/// in parallel. By default all CPU cores are used, use
/// primesieve::set_num_threads(int) to change the number of
/// threads.
///
uint64_t parallel_count_triplets(uint64_t start, uint64_t stop);

/// Count the prime quadruplets within the interval [start, stop]
/// in parallel. By default all CPU cores are used, use
/// primesieve::set_num_threads(int) to change the number of
/// threads.
///
uint64_t parallel_count_quadruplets(uint64_t start, uint64_t stop);

/// Count the prime quintuplets within the interval [start, stop]
/// in parallel. By default all CPU cores are used, use
/// primesieve::set_num_threads(int) to change the number of
/// threads.
///
uint64_t parallel_count_quintuplets(uint64_t start, uint64_t stop);

/// Count the prime sextuplets within the interval [start, stop] in
/// parallel. By default all CPU cores are used, use
/// primesieve::set_num_threads(int) to change the number of
/// threads.
///
uint64_t parallel_count_sextuplets(uint64_t start, uint64_t stop);

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

/// Call back the primes within the interval [start, stop].
/// @param callback  A callback function.
///
void callback_primes(uint64_t start, uint64_t stop, void (*callback)(uint64_t prime));

/// Call back the primes within the interval [start, stop].
/// @param callback  An object derived from primesieve::Callback<uint64_t>.
///
void callback_primes(uint64_t start, uint64_t stop, primesieve::Callback<uint64_t>* callback);

/// Get the current set sieve size in kilobytes.
int get_sieve_size();

/// Get the current set number of threads.
int get_num_threads();

/// Returns the largest valid stop number for primesieve.
/// @return 2^64-1 (UINT64_MAX).
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
/// primesieve::parallel_* function calls.
///
void set_num_threads(int num_threads);

/// Run extensive correctness tests.
/// The tests last about one minute on a quad core CPU from
/// 2013 and use up to 1 gigabyte of memory.
/// @return true if success else false.
///
bool primesieve_test();

/// Get the primesieve version number, in the form “i.j.k”.
std::string primesieve_version();

}

#endif
