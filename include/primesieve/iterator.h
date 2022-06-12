/**
 * @file   iterator.h
 * @brief  primesieve_iterator allows to easily iterate over primes
 *         both forwards and backwards. Generating the first prime
 *         has a complexity of O(r log log r) operations with
 *         r = n^0.5, after that any additional prime is generated in
 *         amortized O(log n log log n) operations. The memory usage
 *         is about PrimePi(n^0.5) * 8 bytes.
 *
 *         The @link primesieve_iterator.c primesieve_iterator.c
 *         @endlink example shows how to use primesieve_iterator.
 *         If any error occurs primesieve_next_prime() and
 *         primesieve_prev_prime() return PRIMESIEVE_ERROR.
 *         Furthermore primesieve_iterator.is_error is initialized
 *         to 0 and set to 1 if any error occurs.
 *
 * Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
 *
 * This file is distributed under the BSD License. See the COPYING
 * file in the top level directory.
 */

#ifndef PRIMESIEVE_ITERATOR_H
#define PRIMESIEVE_ITERATOR_H

#include <stdint.h>
#include <stddef.h>

#if __cplusplus >= 202002L && \
    defined(__has_cpp_attribute)
  #if __has_cpp_attribute(unlikely)
    #define IF_UNLIKELY_PRIMESIEVE(x) if (x) [[unlikely]]
  #endif
#elif defined(__has_builtin)
  #if __has_builtin(__builtin_expect)
    #define IF_UNLIKELY_PRIMESIEVE(x) if (__builtin_expect(!!(x), 0))
  #endif
#endif
#if !defined(IF_UNLIKELY_PRIMESIEVE)
  #define IF_UNLIKELY_PRIMESIEVE(x) if (x)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * C prime iterator, please refer to @link iterator.h iterator.h
 * @endlink for more information.
 */
typedef struct
{
  size_t i;
  size_t size;
  uint64_t start;
  uint64_t stop_hint;
  uint64_t* primes;
  void* memory;
  int is_error;
} primesieve_iterator;

/** Initialize the primesieve iterator before first using it */
void primesieve_init(primesieve_iterator* it);

/** Free all memory */
void primesieve_free_iterator(primesieve_iterator* it);

/**
 * Frees most memory, but keeps some smaller data structures
 * (e.g. the PreSieve object) that are useful if the
 * primesieve_iterator is reused. The remaining memory
 * uses at most 200 kilobytes.
 */
void primesieve_clear(primesieve_iterator* it);

/**
 * Reset the primesieve iterator to start.
 * @param start      Generate primes > start (or < start).
 * @param stop_hint  Stop number optimization hint. E.g. if you want
 *                   to generate the primes below 1000 use
 *                   stop_hint = 1000, if you don't know use
 *                   UINT64_MAX.
 */
void primesieve_skipto(primesieve_iterator* it, uint64_t start, uint64_t stop_hint);

/** Internal use */
void primesieve_generate_next_primes(primesieve_iterator*);

/** Internal use */
void primesieve_generate_prev_primes(primesieve_iterator*);

/**
 * Get the next prime.
 * Returns UINT64_MAX if next prime > 2^64.
 */
static inline uint64_t primesieve_next_prime(primesieve_iterator* it)
{
  IF_UNLIKELY_PRIMESIEVE(++(it->i) >= it->size)
    primesieve_generate_next_primes(it);
  return it->primes[it->i];
}

/**
 * Get the previous prime.
 * primesieve_prev_prime(n) returns 0 for n <= 2.
 * Note that primesieve_next_prime() runs up to 2x faster than
 * primesieve_prev_prime(). Hence if the same algorithm can be written
 * using either primesieve_prev_prime() or primesieve_next_prime()
 * it is preferable to use primesieve_next_prime().
 */
static inline uint64_t primesieve_prev_prime(primesieve_iterator* it)
{
  IF_UNLIKELY_PRIMESIEVE(it->i-- == 0)
    primesieve_generate_prev_primes(it);
  return it->primes[it->i];
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
