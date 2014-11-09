/**
 *  @file   primesieve.h
 *  @brief  primesieve C API. primesieve is a library for fast prime
 *          number generation. In case of an error all functions set
 *          errno to EDOM and functions that have a uint64_t return
 *          type return UINT64_MAX (= PRIMESIEVE_ERROR).
 * 
 *  Copyright (C) 2014 Kim Walisch, <kim.walisch@gmail.com>
 * 
 *  This file is distributed under the BSD License. See the COPYING
 *  file in the top level directory.
 */

#ifndef PRIMESIEVE_H
#define PRIMESIEVE_H

#define PRIMESIEVE_VERSION "5.4.1"
#define PRIMESIEVE_VERSION_MAJOR 5
#define PRIMESIEVE_VERSION_MINOR 4
#define PRIMESIEVE_VERSION_PATCH 1

#include <primesieve/primesieve_iterator.h>
#include <stdint.h>
#include <stddef.h>

/** primesieve functions return PRIMESIEVE_ERROR
 *  (UINT64_MAX) if any error occurs.
 */
#define PRIMESIEVE_ERROR ((uint64_t)~((uint64_t)0))

#ifdef __cplusplus
extern "C" {
#endif

enum {
  /** Use all CPU cores for prime sieving. */
  MAX_THREADS = - 1,
  /** Generate primes of short type. */
  SHORT_PRIMES,
  /** Generate primes of unsigned short type. */
  USHORT_PRIMES,
  /** Generate primes of int type. */
  INT_PRIMES,
  /** Generate primes of unsigned int type. */
  UINT_PRIMES,
  /** Generate primes of long type. */
  LONG_PRIMES,
  /** Generate primes of unsigned long type. */
  ULONG_PRIMES,
  /** Generate primes of long long type. */
  LONGLONG_PRIMES,
  /** Generate primes of unsigned long long type. */
  ULONGLONG_PRIMES,
  /** Generate primes of int16_t type. */
  INT16_PRIMES,
  /** Generate primes of uint16_t type. */
  UINT16_PRIMES,
  /** Generate primes of int32_t type. */
  INT32_PRIMES,
  /** Generate primes of uint32_t type. */
  UINT32_PRIMES,
  /** Generate primes of int64_t type. */
  INT64_PRIMES,
  /** Generate primes of uint64_t type. */
  UINT64_PRIMES
};

/** Get an array with the primes inside the interval [start, stop].
 *  @param size  The size of the returned primes array.
 *  @param type  The type of the primes to generate, e.g. INT_PRIMES.
 *  @pre stop <= 2^64 - 2^32 * 10.
 */
void* primesieve_generate_primes(uint64_t start, uint64_t stop, size_t* size, int type);

/** Get an array with the first n primes >= start.
 *  @param type  The type of the primes to generate, e.g. INT_PRIMES.
 *  @pre stop <= 2^64 - 2^32 * 10.
 */
void* primesieve_generate_n_primes(uint64_t n, uint64_t start, int type);

/** Find the nth prime.
 *  @param n  if n = 0 finds the 1st prime >= start, <br/>
 *            if n > 0 finds the nth prime > start, <br/>
 *            if n < 0 finds the nth prime < start (backwards).
 *  @pre   start <= 2^64 - 2^32 * 11.
 */
uint64_t primesieve_nth_prime(int64_t n, uint64_t start);

/** Find the nth prime in parallel.
 *  By default all CPU cores are used, use
 *  primesieve_set_num_threads(int) to change the number of threads.
 *  @param n  if n = 0 finds the 1st prime >= start, <br/>
 *            if n > 0 finds the nth prime > start, <br/>
 *            if n < 0 finds the nth prime < start (backwards).
 *  @pre   start <= 2^64 - 2^32 * 11.
 */
uint64_t primesieve_parallel_nth_prime(int64_t n, uint64_t start);

/** Count the primes within the interval [start, stop].
 *  @pre stop <= 2^64 - 2^32 * 10.
 */
uint64_t primesieve_count_primes(uint64_t start, uint64_t stop);

/** Count the twin primes within the interval [start, stop].
 *  @pre stop <= 2^64 - 2^32 * 10.
 */
uint64_t primesieve_count_twins(uint64_t start, uint64_t stop);

/** Count the prime triplets within the interval [start, stop].
 *  @pre stop <= 2^64 - 2^32 * 10.
 */
uint64_t primesieve_count_triplets(uint64_t start, uint64_t stop);

/** Count the prime quadruplets within the interval [start, stop].
 *  @pre stop <= 2^64 - 2^32 * 10.
 */
uint64_t primesieve_count_quadruplets(uint64_t start, uint64_t stop);

/** Count the prime quintuplets within the interval [start, stop].
 *  @pre stop <= 2^64 - 2^32 * 10.
 */
uint64_t primesieve_count_quintuplets(uint64_t start, uint64_t stop);

/** Count the prime sextuplets within the interval [start, stop].
 *  @pre stop <= 2^64 - 2^32 * 10.
 */
uint64_t primesieve_count_sextuplets(uint64_t start, uint64_t stop);

/** Count the primes within the interval [start, stop] in
 *  parallel. By default all CPU cores are used, use
 *  primesieve_set_num_threads(int) to change the number of threads.
 *  @pre stop <= 2^64 - 2^32 * 10.
 */
uint64_t primesieve_parallel_count_primes(uint64_t start, uint64_t stop);

/** Count the twin primes within the interval [start, stop] in
 *  parallel. By default all CPU cores are used, use
 *  primesieve_set_num_threads(int) to change the number of threads.
 *  @pre stop <= 2^64 - 2^32 * 10.
 */
uint64_t primesieve_parallel_count_twins(uint64_t start, uint64_t stop);

/** Count the prime triplets within the interval [start, stop] in
 *  parallel. By default all CPU cores are used, use
 *  primesieve_set_num_threads(int) to change the number of threads.
 *  @pre stop <= 2^64 - 2^32 * 10.
 */
uint64_t primesieve_parallel_count_triplets(uint64_t start, uint64_t stop);

/** Count the prime quadruplets within the interval [start, stop] in
 *  parallel. By default all CPU cores are used, use
 *  primesieve_set_num_threads(int) to change the number of threads.
 *  @pre stop <= 2^64 - 2^32 * 10.
 */
uint64_t primesieve_parallel_count_quadruplets(uint64_t start, uint64_t stop);

/** Count the prime quintuplets within the interval [start, stop] in
 *  parallel. By default all CPU cores are used, use
 *  primesieve_set_num_threads(int) to change the number of threads.
 *  @pre stop <= 2^64 - 2^32 * 10.
 */
uint64_t primesieve_parallel_count_quintuplets(uint64_t start, uint64_t stop);

/** Count the prime sextuplets within the interval [start, stop] in
 *  parallel. By default all CPU cores are used, use
 *  primesieve_set_num_threads(int) to change the number of threads.
 *  @pre stop <= 2^64 - 2^32 * 10.
 */
uint64_t primesieve_parallel_count_sextuplets(uint64_t start, uint64_t stop);

/** Print the primes within the interval [start, stop]
 *  to the standard output.
 *  @pre stop <= 2^64 - 2^32 * 10.
 */
void primesieve_print_primes(uint64_t start, uint64_t stop);

/** Print the twin primes within the interval [start, stop]
 *  to the standard output.
 *  @pre stop <= 2^64 - 2^32 * 10.
 */
void primesieve_print_twins(uint64_t start, uint64_t stop);

/** Print the prime triplets within the interval [start, stop]
 *  to the standard output.
 *  @pre stop <= 2^64 - 2^32 * 10.
 */
void primesieve_print_triplets(uint64_t start, uint64_t stop);

/** Print the prime quadruplets within the interval [start, stop]
 *  to the standard output.
 *  @pre stop <= 2^64 - 2^32 * 10.
 */
void primesieve_print_quadruplets(uint64_t start, uint64_t stop);

/** Print the prime quintuplets within the interval [start, stop]
 *  to the standard output.
 *  @pre stop <= 2^64 - 2^32 * 10.
 */
void primesieve_print_quintuplets(uint64_t start, uint64_t stop);

/** Print the prime sextuplets within the interval [start, stop]
 *  to the standard output.
 *  @pre stop <= 2^64 - 2^32 * 10.
 */
void primesieve_print_sextuplets(uint64_t start, uint64_t stop);

/** Call back the primes within the interval [start, stop].
 *  @param callback  A callback function.
 *  @pre stop <= 2^64 - 2^32 * 10.
 */
void primesieve_callback_primes(uint64_t start, uint64_t stop, void (*callback)(uint64_t prime));

/** Call back the primes within the interval [start, stop] in
 *  parallel. This function is not synchronized, multiple threads call
 *  back primes in parallel. By default all CPU cores are used, use
 *  primesieve_set_num_threads(int) to change the number of threads.
 *  @warning         Primes are not called back in arithmetic order.
 *  @param callback  A callback function.
 *  @pre   stop      <= 2^64 - 2^32 * 10.
 */
void primesieve_parallel_callback_primes(uint64_t start, uint64_t stop, void (*callback)(uint64_t prime, int thread_id));

/** Get the current set sieve size in kilobytes. */
int primesieve_get_sieve_size();

/** Get the current set number of threads.
 *  @note By default MAX_THREADS (-1) is returned.
 */
int primesieve_get_num_threads();

/** Returns the largest valid stop number for primesieve.
 *  @return (2^64-1) - (2^32-1) * 10.
 */
uint64_t primesieve_get_max_stop();

/** Set the sieve size in kilobytes.
 *  The best sieving performance is achieved with a sieve size of
 *  your CPU's L1 data cache size (per core). For sieving >= 10^17 a
 *  sieve size of your CPU's L2 cache size sometimes performs
 *  better.
 *  @param sieve_size Sieve size in kilobytes.
 *  @pre   sieve_size >= 1 && <= 2048.
 */
void primesieve_set_sieve_size(int sieve_size);

/** Set the number of threads for use in subsequent
 *  primesieve_parallel_* function calls. Note that this only
 *  changes the number of threads for the current process.
 *  @param num_threads  Number of threads for sieving
 *                      or MAX_THREADS to use all CPU cores.
 */
void primesieve_set_num_threads(int num_threads);

/** Deallocate a primes array created using the
 *  primesieve_generate_primes() or primesieve_generate_n_primes()
 *  functions.
 */
void primesieve_free(void* primes);

/** Run extensive correctness tests.
 *  The tests last about one minute on a quad core CPU from
 *  2013 and use up to 1 gigabyte of memory.
 *  @return 1 if success, 0 if error.
 */
int primesieve_test();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
