/**
 * @file   primesieve.h
 * @brief  primesieve C API. primesieve is a library for quickly
 *         generating prime numbers. If an error occurs, primesieve
 *         functions with a uint64_t return type return PRIMESIEVE_ERROR
 *         and the corresponding error message is printed to the
 *         standard error stream. libprimesieve also sets the C errno
 *         variable to EDOM if an error occurs.
 * 
 * Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
 * 
 * This file is distributed under the BSD License.
 */

#ifndef PRIMESIEVE_H
#define PRIMESIEVE_H

#define PRIMESIEVE_VERSION "12.10"
#define PRIMESIEVE_VERSION_MAJOR 12
#define PRIMESIEVE_VERSION_MINOR 10

#include <primesieve/iterator.h>

#include <stdint.h>
#include <stddef.h>

/**
 * primesieve functions return PRIMESIEVE_ERROR
 * (UINT64_MAX) if any error occurs.
 */
#define PRIMESIEVE_ERROR ((uint64_t) ~((uint64_t) 0))

#ifdef __cplusplus
extern "C" {
#endif

enum {
  /** Generate primes of short type */
  SHORT_PRIMES,
  /** Generate primes of unsigned short type */
  USHORT_PRIMES,
  /** Generate primes of int type */
  INT_PRIMES,
  /** Generate primes of unsigned int type */
  UINT_PRIMES,
  /** Generate primes of long type */
  LONG_PRIMES,
  /** Generate primes of unsigned long type */
  ULONG_PRIMES,
  /** Generate primes of long long type */
  LONGLONG_PRIMES,
  /** Generate primes of unsigned long long type */
  ULONGLONG_PRIMES,
  /** Generate primes of int16_t type */
  INT16_PRIMES,
  /** Generate primes of uint16_t type */
  UINT16_PRIMES,
  /** Generate primes of int32_t type */
  INT32_PRIMES,
  /** Generate primes of uint32_t type */
  UINT32_PRIMES,
  /** Generate primes of int64_t type */
  INT64_PRIMES,
  /** Generate primes of uint64_t type */
  UINT64_PRIMES
};

/**
 * Get an array with the primes inside the interval [start, stop].
 * @param size  The size of the returned primes array.
 * @param type  The type of the primes to generate, e.g. INT_PRIMES.
 * 
 * In case an error occurs the error message is printed to the
 * standard error stream, the size is set to 0 and a NULL pointer
 * is returned. In order to distinguish an "error" from "no primes
 * found within [start, stop]" libprimesieve also sets the C errno
 * variable (from <errno.h>) to EDOM if any error occurs. By
 * checking errno after calling primesieve_generate_primes() users
 * can reliably detect errors.
 */
void* primesieve_generate_primes(uint64_t start, uint64_t stop, size_t* size, int type);

/**
 * Get an array with the first n primes >= start.
 * @param type  The type of the primes to generate, e.g. INT_PRIMES.
 * 
 * In case an error occurs the error message is printed to the
 * standard error stream and a NULL pointer is returned.
 * libprimesieve also sets the C errno variable (from <errno.h>)
 * to EDOM if any error occurs. The only advantage which checking
 * errno (after primesieve_generate_n_primes()) has over checking
 * if a NULL pointer has been returned, is that errno is not set
 * when calling primesieve_generate_n_primes(0, start, type) which
 * is valid (but useless) and which returns a NULL pointer.
 */
void* primesieve_generate_n_primes(uint64_t n, uint64_t start, int type);

/**
 * Find the nth prime.
 * By default all CPU cores are used, use
 * primesieve_set_num_threads(int threads) to change the
 * number of threads.
 *
 * Note that each call to primesieve_nth_prime(n, start) incurs an
 * initialization overhead of O(sqrt(start)) even if n is tiny.
 * Hence it is not a good idea to use primesieve_nth_prime()
 * repeatedly in a loop to get the next (or previous) prime. For
 * this use case it is better to use a primesieve::iterator which
 * needs to be initialized only once.
 *
 * @param n  if n = 0 finds the 1st prime >= start, <br/>
 *           if n > 0 finds the nth prime > start, <br/>
 *           if n < 0 finds the nth prime < start (backwards).
 */
uint64_t primesieve_nth_prime(int64_t n, uint64_t start);

/**
 * Count the primes within the interval [start, stop].
 * By default all CPU cores are used, use
 * primesieve_set_num_threads(int threads) to change the
 * number of threads.
 *
 * Note that each call to primesieve_count_primes() incurs an
 * initialization overhead of O(sqrt(stop)) even if the interval
 * [start, stop] is tiny. Hence if you have written an algorithm
 * that makes many calls to primesieve_count_primes() it may be
 * preferable to use a primesieve::iterator which needs to be
 * initialized only once.
 */
uint64_t primesieve_count_primes(uint64_t start, uint64_t stop);

/**
 * Count the twin primes within the interval [start, stop].
 * By default all CPU cores are used, use
 * primesieve_set_num_threads(int threads) to change the
 * number of threads.
 */
uint64_t primesieve_count_twins(uint64_t start, uint64_t stop);

/**
 * Count the prime triplets within the interval [start, stop].
 * By default all CPU cores are used, use
 * primesieve_set_num_threads(int threads) to change the
 * number of threads.
 */
uint64_t primesieve_count_triplets(uint64_t start, uint64_t stop);

/**
 * Count the prime quadruplets within the interval [start, stop]. 
 * By default all CPU cores are used, use
 * primesieve_set_num_threads(int threads) to change the
 * number of threads.
 */
uint64_t primesieve_count_quadruplets(uint64_t start, uint64_t stop);

/**
 * Count the prime quintuplets within the interval [start, stop].
 * By default all CPU cores are used, use
 * primesieve_set_num_threads(int threads) to change the
 * number of threads.
 */
uint64_t primesieve_count_quintuplets(uint64_t start, uint64_t stop);

/**
 * Count the prime sextuplets within the interval [start, stop].
 * By default all CPU cores are used, use
 * primesieve_set_num_threads(int threads) to change the
 * number of threads.
 */
uint64_t primesieve_count_sextuplets(uint64_t start, uint64_t stop);

/**
 * Print the primes within the interval [start, stop]
 * to the standard output.
 */
void primesieve_print_primes(uint64_t start, uint64_t stop);

/**
 * Print the twin primes within the interval [start, stop]
 * to the standard output.
 */
void primesieve_print_twins(uint64_t start, uint64_t stop);

/**
 * Print the prime triplets within the interval [start, stop]
 * to the standard output.
 */
void primesieve_print_triplets(uint64_t start, uint64_t stop);

/**
 * Print the prime quadruplets within the interval [start, stop]
 * to the standard output.
 */
void primesieve_print_quadruplets(uint64_t start, uint64_t stop);

/**
 * Print the prime quintuplets within the interval [start, stop]
 * to the standard output.
 */
void primesieve_print_quintuplets(uint64_t start, uint64_t stop);

/**
 * Print the prime sextuplets within the interval [start, stop]
 * to the standard output.
 */
void primesieve_print_sextuplets(uint64_t start, uint64_t stop);

/**
 * Returns the largest valid stop number for primesieve.
 * @return 2^64-1 (UINT64_MAX).
 */
uint64_t primesieve_get_max_stop(void);

/** Get the current set sieve size in KiB */
int primesieve_get_sieve_size(void);

/** Get the current set number of threads */
int primesieve_get_num_threads(void);

/**
 * Set the sieve size in KiB (kibibyte).
 * The best sieving performance is achieved with a sieve size
 * of your CPU's L1 or L2 cache size (per core).
 * @pre sieve_size >= 16 && <= 8192.
 */
void primesieve_set_sieve_size(int sieve_size);

/**
 * Set the number of threads for use in
 * primesieve_count_*() and primesieve_nth_prime().
 * By default all CPU cores are used.
 */
void primesieve_set_num_threads(int num_threads);

/**
 * Deallocate a primes array created using the
 * primesieve_generate_primes() or primesieve_generate_n_primes()
 * functions.
 */
void primesieve_free(void* primes);

/** Get the primesieve version number, in the form “i.j” */
const char* primesieve_version(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
