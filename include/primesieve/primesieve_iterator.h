/**
 *  @file   primesieve_iterator.h
 *  @brief  Fast prime iterator for use in C code. The @link
 *          primesieve_iterator.c primesieve_iterator.c @endlink
 *          example shows how to use primesieve_iterator. If any error
 *          occurs errno is is set to EDOM and primesieve_next_prime()
 *          and primesieve_previous_prime() return PRIMESIEVE_ERROR
 *          (UINT64_MAX).
 * 
 *  Copyright (C) 2014 Kim Walisch, <kim.walisch@gmail.com>
 * 
 *  This file is distributed under the BSD License. See the COPYING
 *  file in the top level directory.
 */

#ifndef PRIMESIEVE_ITERATOR_H
#define PRIMESIEVE_ITERATOR_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** C data structure. Please refer to @link primesieve_iterator.h
 *  primesieve_iterator.h @endlink for more information.
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
  uint64_t count_;
  int is_error_;
} primesieve_iterator;

/** Initialize the primesieve iterator before first using it. */
void primesieve_init(primesieve_iterator* pi);

/** Free all memory. */
void primesieve_free_iterator(primesieve_iterator* pi);

/** Set the primesieve iterator to start.
 *  @param start      Start generating primes at this number.
 *  @param stop_hint  Stop number optimization hint. E.g. if you want
 *                    to generate the primes below 1000 use
 *                    stop_hint = 1000, if you don't know use
 *                    primesieve_get_max_stop().
 *  @pre   start      <= 2^64 - 2^32 * 10
 */
void primesieve_skipto(primesieve_iterator* pi, uint64_t start, uint64_t stop_hint);

/** Internal use. */
void primesieve_generate_next_primes(primesieve_iterator*);

/** Internal use. */
void primesieve_generate_previous_primes(primesieve_iterator*);

/** Advance the primesieve iterator by one position.
 *  @return  The next prime.
 */
static inline uint64_t primesieve_next_prime(primesieve_iterator* pi)
{
  if (pi->i_++ == pi->last_idx_)
    primesieve_generate_next_primes(pi);
  return pi->primes_[pi->i_];
}

/** Decrease the primesieve iterator by one position.
 *  @return  The previous prime.
 */
static inline uint64_t primesieve_previous_prime(primesieve_iterator* pi)
{
  if (pi->i_-- == 0)
    primesieve_generate_previous_primes(pi);
  return pi->primes_[pi->i_];
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
