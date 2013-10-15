/**
 *  @file   primesieve_iterator.h
 *  @brief  Fast prime iterator for use in C code. The @link
 *          next_prime.c next_prime.c @endlink example shows how to
 *          use primesieve_iterator. If any error occurs errno is
 *          is set to EDOM and primesieve_prime(), primesieve_next()
 *          and primesieve_previous() return UINT64_MAX.
 * 
 *  Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
 * 
 *  This file is distributed under the BSD License. See the COPYING
 *  file in the top level directory.
 */

#ifndef PRIMESIEVE_ITERATOR_C_H
#define PRIMESIEVE_ITERATOR_C_H

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
  size_t size_;
  uint64_t* primes_;
  uint64_t* primes_pimpl_;
  uint64_t start_;
  uint64_t count_;
  int first_;
  int adjust_skipto_;
} primesieve_iterator;

/** Internal use only. */
void generate_next_primes(primesieve_iterator*);
void generate_previous_primes(primesieve_iterator*);

/** Initialize the primesieve iterator before first using it. */
void primesieve_init(primesieve_iterator* pi);

/** Free all memory. */
void primesieve_free(primesieve_iterator* pi);

/** Set the primesieve iterator to start.
 *  @pre  start <= 2^64 - 2^32 * 10
 */
void primesieve_skipto(primesieve_iterator* pi, uint64_t start);

/** Get the current prime. */
static inline uint64_t primesieve_prime(primesieve_iterator* pi)
{
  if (pi->first_)
    generate_next_primes(pi);
  return pi->primes_[pi->i_];
}

/** Advance the primesieve iterator by one position.
 *  @return  The next prime.
 */
static inline uint64_t primesieve_next(primesieve_iterator* pi)
{
  if (++pi->i_ >= pi->size_ || pi->first_)
    generate_next_primes(pi);
  return pi->primes_[pi->i_];
}

/** Decrease the primesieve iterator by one position.
 *  @return  The previous prime.
 */
static inline uint64_t primesieve_previous(primesieve_iterator* pi)
{
  if (pi->i_ == 0 || pi->first_)
    generate_previous_primes(pi);
  return pi->primes_[--pi->i_];
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
