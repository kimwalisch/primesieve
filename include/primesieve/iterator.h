/**
 * @file   iterator.h
 * @brief  primesieve_iterator allows to easily iterate over primes
 *         both forwards and backwards. Generating the first prime
 *         has a complexity of O(r log log r) operations with
 *         r = n^0.5, after that any additional prime is generated in
 *         amortized O(log n log log n) operations. The memory usage
 *         is about pi(n^0.5) * 16 bytes.
 *
 *         The @link primesieve_iterator.c primesieve_iterator.c
 *         @endlink example shows how to use primesieve_iterator. If
 *         any error occurs errno is set to EDOM and
 *         primesieve_next_prime() and primesieve_prev_prime()
 *         return PRIMESIEVE_ERROR.
 * 
 * Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
 *
 * This file is distributed under the BSD License. See the COPYING
 * file in the top level directory.
 */

#ifndef PRIMESIEVE_ITERATOR_H
#define PRIMESIEVE_ITERATOR_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * C prime iterator, please refer to @link iterator.h iterator.h
 * @endlink for more information.
 */
typedef struct
{
  size_t i_;
  size_t last_idx_;
  uint64_t* primes_;
  uint64_t* primes_pimpl_;
  uint64_t start_;
  uint64_t stop_;
  uint64_t stop_hint_;
  uint64_t tiny_cache_size_;
  int is_error_;
} primesieve_iterator;

/** Initialize the primesieve iterator before first using it */
void primesieve_init(primesieve_iterator* it);

/** Free all memory */
void primesieve_free_iterator(primesieve_iterator* it);

/**
 * Set the primesieve iterator to start.
 * @param start      Generate primes > start (or < start).
 * @param stop_hint  Stop number optimization hint. E.g. if you want
 *                   to generate the primes below 1000 use
 *                   stop_hint = 1000, if you don't know use
 *                   primesieve_get_max_stop().
 */
void primesieve_skipto(primesieve_iterator* it, uint64_t start, uint64_t stop_hint);

/** Internal use */
void primesieve_generate_next_primes(primesieve_iterator*);

/** Internal use */
void primesieve_generate_prev_primes(primesieve_iterator*);

/** Get the next prime */
static inline uint64_t primesieve_next_prime(primesieve_iterator* it)
{
  if (it->i_++ == it->last_idx_)
    primesieve_generate_next_primes(it);
  return it->primes_[it->i_];
}

/**
 * Get the previous prime,
 * or 0 if input <= 2 e.g. prev_prime(2) = 0.
 */
static inline uint64_t primesieve_prev_prime(primesieve_iterator* it)
{
  if (it->i_-- == 0)
    primesieve_generate_prev_primes(it);
  return it->primes_[it->i_];
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
