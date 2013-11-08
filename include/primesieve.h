/**
 *  @file   primesieve.h
 *  @brief  primesieve C API. primesieve is a library for fast prime
 *          number generation. In case of an error all functions set
 *          errno to EDOM and functions that have a uint64_t return
 *          type return UINT64_MAX (= PRIMESIEVE_ERROR).
 * 
 *  Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
 * 
 *  This file is distributed under the BSD License. See the COPYING
 *  file in the top level directory.
 */

#ifndef PRIMESIEVE_H
#define PRIMESIEVE_H

#define PRIMESIEVE_VERSION "5.0"
#define PRIMESIEVE_VERSION_MAJOR 5
#define PRIMESIEVE_VERSION_MINOR 0
#define PRIMESIEVE_YEAR "2013"

#include <primesieve/soe/primesieve_iterator.h>
#include <stdint.h>
#include <stddef.h>

/** primesieve functions return UINT64_MAX if any error occurs. */
#define PRIMESIEVE_ERROR ((uint64_t)~((uint64_t)0))

#ifdef __cplusplus
extern "C" {
#endif

enum {
  /** Use all CPU cores for prime sieving. */
  MAX_THREADS,
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
void* generate_primes(uint64_t start, uint64_t stop, size_t* size, int type);

/** Get an array with the first n primes >= start.
 *  @param type  The type of the primes to generate, e.g. INT_PRIMES.
 *  @pre stop <= 2^64 - 2^32 * 10.
 */
void* generate_n_primes(uint64_t n, uint64_t start, int type);

/** Deallocate a primes array created using the generate_primes()
 *  or generate_n_primes() functions.
 */
void primesieve_free(void* primes);

/** Find the nth prime.
 *  @param start  Start nth prime search at this offset.
 *  @pre   start  <= 2^64 - 2^32 * 10.
 */
uint64_t nth_prime(uint64_t n, uint64_t start);

/** Find the nth prime.
 *  @param start    Start nth prime search at this offset.
 *  @param threads  Number of threads.
 *  @pre   start    <= 2^64 - 2^32 * 10.
 */
uint64_t parallel_nth_prime(uint64_t n, uint64_t start, int threads);

/** Count the primes within the interval [start, stop].
 *  @pre stop <= 2^64 - 2^32 * 10.
 */
uint64_t count_primes(uint64_t start, uint64_t stop);

/** Count the twin primes within the interval [start, stop].
 *  @pre stop <= 2^64 - 2^32 * 10.
 */
uint64_t count_twins(uint64_t start, uint64_t stop);

/** Count the prime triplets within the interval [start, stop].
 *  @pre stop <= 2^64 - 2^32 * 10.
 */
uint64_t count_triplets(uint64_t start, uint64_t stop);

/** Count the prime quadruplets within the interval [start, stop].
 *  @pre stop <= 2^64 - 2^32 * 10.
 */
uint64_t count_quadruplets(uint64_t start, uint64_t stop);

/** Count the prime quintuplets within the interval [start, stop].
 *  @pre stop <= 2^64 - 2^32 * 10.
 */
uint64_t count_quintuplets(uint64_t start, uint64_t stop);

/** Count the prime sextuplets within the interval [start, stop].
 *  @pre stop <= 2^64 - 2^32 * 10.
 */
uint64_t count_sextuplets(uint64_t start, uint64_t stop);

/** Count the prime septuplets within the interval [start, stop].
 *  @pre stop <= 2^64 - 2^32 * 10.
 */
uint64_t count_septuplets(uint64_t start, uint64_t stop);

/** Count the primes within the interval [start, stop].
 *  @param threads  Number of threads.
 *  @pre   stop     <= 2^64 - 2^32 * 10.
 */
uint64_t parallel_count_primes(uint64_t start, uint64_t stop, int threads);

/** Count the twin primes within the interval [start, stop].
 *  @param threads  Number of threads.
 *  @pre   stop     <= 2^64 - 2^32 * 10.
 */
uint64_t parallel_count_twins(uint64_t start, uint64_t stop, int threads);

/** Count the prime triplets within the interval [start, stop].
 *  @param threads  Number of threads.
 *  @pre   stop     <= 2^64 - 2^32 * 10.
 */
uint64_t parallel_count_triplets(uint64_t start, uint64_t stop, int threads);

/** Count the prime quadruplets within the interval [start, stop].
 *  @param threads  Number of threads.
 *  @pre   stop     <= 2^64 - 2^32 * 10.
 */
uint64_t parallel_count_quadruplets(uint64_t start, uint64_t stop, int threads);

/** Count the prime quintuplets within the interval [start, stop].
 *  @param threads  Number of threads.
 *  @pre   stop     <= 2^64 - 2^32 * 10.
 */
uint64_t parallel_count_quintuplets(uint64_t start, uint64_t stop, int threads);

/** Count the prime sextuplets within the interval [start, stop].
 *  @param threads  Number of threads.
 *  @pre   stop     <= 2^64 - 2^32 * 10.
 */
uint64_t parallel_count_sextuplets(uint64_t start, uint64_t stop, int threads);

/** Count the prime septuplets within the interval [start, stop].
 *  @param threads  Number of threads.
 *  @pre   stop     <= 2^64 - 2^32 * 10.
 */
uint64_t parallel_count_septuplets(uint64_t start, uint64_t stop, int threads);

/** Print the primes within the interval [start, stop]
 *  to the standard output.
 *  @pre stop <= 2^64 - 2^32 * 10.
 */
void print_primes(uint64_t start, uint64_t stop);

/** Print the twin primes within the interval [start, stop]
 *  to the standard output.
 *  @pre stop <= 2^64 - 2^32 * 10.
 */
void print_twins(uint64_t start, uint64_t stop);

/** Print the prime triplets within the interval [start, stop]
 *  to the standard output.
 *  @pre stop <= 2^64 - 2^32 * 10.
 */
void print_triplets(uint64_t start, uint64_t stop);

/** Print the prime quadruplets within the interval [start, stop]
 *  to the standard output.
 *  @pre stop <= 2^64 - 2^32 * 10.
 */
void print_quadruplets(uint64_t start, uint64_t stop);

/** Print the prime quintuplets within the interval [start, stop]
 *  to the standard output.
 *  @pre stop <= 2^64 - 2^32 * 10.
 */
void print_quintuplets(uint64_t start, uint64_t stop);

/** Print the prime sextuplets within the interval [start, stop]
 *  to the standard output.
 *  @pre stop <= 2^64 - 2^32 * 10.
 */
void print_sextuplets(uint64_t start, uint64_t stop);

/** Print the prime septuplets within the interval [start, stop]
 *  to the standard output.
 *  @pre stop <= 2^64 - 2^32 * 10.
 */
void print_septuplets(uint64_t start, uint64_t stop);

/** Call back the primes within the interval [start, stop].
 *  @param callback  A callback function.
 *  @pre   stop <= 2^64 - 2^32 * 10.
 */
void callback_primes(uint64_t start, uint64_t stop, void (*callback)(uint64_t prime));

/** Call back the primes within the interval [start, stop].
 *  This function is not synchronized, multiple threads call back
 *  primes in parallel.
 *  @warning         Primes are not called back in arithmetic order.
 *  @param callback  A callback function.
 *  @param threads   Number of threads.
 *  @pre   stop      <= 2^64 - 2^32 * 10.
 */
void parallel_callback_primes(uint64_t start, uint64_t stop, void (*callback)(uint64_t prime, int thread_id), int threads);

/** Returns the largest valid stop number for primesieve.
 *  @return (2^64-1) - (2^32-1) * 10.
 */
uint64_t max_stop();

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
