///
/// @file   primesieve.hpp
/// @brief  primesieve C++ API. primesieve is a library for fast
///         prime number generation, in case an error occurs a
///         primesieve::primesieve_error exception (derived form
///         std::runtime_error) is thrown.
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License.
///

#ifndef PRIMESIEVE_HPP
#define PRIMESIEVE_HPP

#define PRIMESIEVE_VERSION "7.2"
#define PRIMESIEVE_VERSION_MAJOR 7
#define PRIMESIEVE_VERSION_MINOR 2

#include <primesieve/iterator.hpp>
#include <primesieve/primesieve_error.hpp>
#include <primesieve/StorePrimes.hpp>

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
    store_primes(0, stop, *primes);
}

/// Store the primes within the interval [start, stop]
/// in the primes vector.
///
template <typename T>
inline void generate_primes(uint64_t start, uint64_t stop, std::vector<T>* primes)
{
  if (primes)
    store_primes(start, stop, *primes);
}

/// Store the first n primes in the primes vector.
template <typename T>
inline void generate_n_primes(uint64_t n, std::vector<T>* primes)
{
  if (primes)
    store_n_primes(n, 0, *primes);
}

/// Store the first n primes >= start in the primes vector.
template <typename T>
inline void generate_n_primes(uint64_t n, uint64_t start, std::vector<T>* primes)
{
  if (primes)
    store_n_primes(n, start, *primes);
}

/// Find the nth prime.
/// By default all CPU cores are used, use
/// primesieve::set_num_threads(int threads) to change the
/// number of threads.
/// @param n  if n = 0 finds the 1st prime >= start, <br/>
///           if n > 0 finds the nth prime > start, <br/>
///           if n < 0 finds the nth prime < start (backwards).
///
uint64_t nth_prime(int64_t n, uint64_t start = 0);

/// Count the primes within the interval [start, stop].
/// By default all CPU cores are used, use
/// primesieve::set_num_threads(int threads) to change the
/// number of threads.
///
uint64_t count_primes(uint64_t start, uint64_t stop);

/// Count the twin primes within the interval [start, stop].
/// By default all CPU cores are used, use
/// primesieve::set_num_threads(int threads) to change the
/// number of threads.
///
uint64_t count_twins(uint64_t start, uint64_t stop);

/// Count the prime triplets within the interval [start, stop].
/// By default all CPU cores are used, use
/// primesieve::set_num_threads(int threads) to change the
/// number of threads.
///
uint64_t count_triplets(uint64_t start, uint64_t stop);

/// Count the prime quadruplets within the interval [start, stop].
/// By default all CPU cores are used, use
/// primesieve::set_num_threads(int threads) to change the
/// number of threads.
///
uint64_t count_quadruplets(uint64_t start, uint64_t stop);

/// Count the prime quintuplets within the interval [start, stop].
/// By default all CPU cores are used, use
/// primesieve::set_num_threads(int threads) to change the
/// number of threads.
///
uint64_t count_quintuplets(uint64_t start, uint64_t stop);

/// Count the prime sextuplets within the interval [start, stop].
/// By default all CPU cores are used, use
/// primesieve::set_num_threads(int threads) to change the
/// number of threads.
///
uint64_t count_sextuplets(uint64_t start, uint64_t stop);

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

/// Returns the largest valid stop number for primesieve.
/// @return 2^64-1 (UINT64_MAX).
///
uint64_t get_max_stop();

/// Get the current set sieve size in KiB.
int get_sieve_size();

/// Get the current set number of threads.
int get_num_threads();

/// Set the sieve size in KiB (kibibyte).
/// The best sieving performance is achieved with a sieve size
/// of your CPU's L1 or L2 cache size (per core).
/// @pre sieve_size >= 8 && <= 4096.
///
void set_sieve_size(int sieve_size);

/// Set the number of threads for use in
/// primesieve::count_*() and primesieve::nth_prime().
/// By default all CPU cores are used.
///
void set_num_threads(int num_threads);

/// Get the primesieve version number, in the form “i.j”.
std::string primesieve_version();

}

#endif
