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
 * Copyright (C) 2024 Kim Walisch, <kim.walisch@gmail.com>
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
  /** Current index of the primes array */
  size_t i;
  /** Current number of primes in the primes array */
  size_t size;
  /** Generate primes >= start */
  uint64_t start;
  /** Generate primes <= stop_hint */
  uint64_t stop_hint;
  /**
   * The primes array.
   * The current smallest prime can be accessed using primes[0].
   * The current largest prime can be accessed using primes[size-1].
   */
  uint64_t* primes;
  /** Pointer to internal IteratorData data structure */
  void* memory;
  /** Initialized to 0, set to 1 if any error occurs */
  int is_error;
} primesieve_iterator;

/** Initialize the primesieve iterator before first using it */
void primesieve_init(primesieve_iterator* it);

/** Free all memory */
void primesieve_free_iterator(primesieve_iterator* it);

/**
 * Reset the start number to 0 and free most memory.
 * Keeps some smaller data structures in memory
 * (e.g. the IteratorData object) that are useful if the
 * primesieve_iterator is reused. The remaining memory
 * uses at most 2 kilobytes.
 */
void primesieve_clear(primesieve_iterator* it);

/**
 * Reset the primesieve iterator to start.
 * @param start      Generate primes >= start (or <= start).
 * @param stop_hint  Stop number optimization hint. E.g. if you want
 *                   to generate the primes <= 1000 use
 *                   stop_hint = 1000, if you don't know use
 *                   UINT64_MAX.
 */
void primesieve_jump_to(primesieve_iterator* it, uint64_t start, uint64_t stop_hint);

/**
 * Reset the primesieve iterator to start.
 * @param start      Generate primes > start (or < start).
 * @param stop_hint  Stop number optimization hint. E.g. if you want
 *                   to generate the primes <= 1000 use
 *                   stop_hint = 1000, if you don't know use
 *                   UINT64_MAX.
 */
#if __STDC_VERSION__ >= 202301
  [[deprecated("Use the new primesieve_jump_to() instead. "
               "Attention: primesieve_jump_to() includes the start number, "
               "whereas primesieve_skipto() excludes the start number. "
               "See: https://github.com/kimwalisch/primesieve/blob/master/doc/C_API.md#primesieve_jump_to-since-primesieve-110")]]
#elif __GNUC__ >= 5
  __attribute__((deprecated(
               "Use the new primesieve_jump_to() instead. "
               "Attention: primesieve_jump_to() includes the start number, "
               "whereas primesieve_skipto() excludes the start number. "
               "See: https://github.com/kimwalisch/primesieve/blob/master/doc/C_API.md#primesieve_jump_to-since-primesieve-110")))
#endif
void primesieve_skipto(primesieve_iterator* it, uint64_t start, uint64_t stop_hint);

/**
 * Used internally by primesieve_next_prime().
 * primesieve_generate_next_primes() fills (overwrites) the primes
 * array with the next few primes (~ 2^10) that are larger than the
 * current largest prime in the primes array or with the
 * primes >= start if the primes array is empty.
 * Note that this function also updates the i & size member variables
 * of the primesieve_iterator struct. The size of the primes array
 * varies, but it is > 0 and usually close to 2^10.
 * If an error occurs primesieve_iterator.is_error is set to 1
 * and the primes array will contain PRIMESIEVE_ERROR.
 */
void primesieve_generate_next_primes(primesieve_iterator*);

/**
 * Used internally by primesieve_prev_prime().
 * primesieve_generate_prev_primes() fills (overwrites) the primes
 * array with the next few primes ~ O(sqrt(n)) that are smaller than
 * the current smallest prime in the primes array or with the
 * primes <= start if the primes array is empty.
 * Note that this function also updates the i & size member variables
 * of the primesieve_iterator struct. The size of the primes array
 * varies, but it is > 0 and ~ O(sqrt(n)).
 * If an error occurs primesieve_iterator.is_error is set to 1
 * and the primes array will contain PRIMESIEVE_ERROR.
 */
void primesieve_generate_prev_primes(primesieve_iterator*);

/**
 * Get the next prime.
 * Returns PRIMESIEVE_ERROR (UINT64_MAX) if any error occrus.
 */
static inline uint64_t primesieve_next_prime(primesieve_iterator* it)
{
  it->i += 1;
  IF_UNLIKELY_PRIMESIEVE(it->i >= it->size)
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
  IF_UNLIKELY_PRIMESIEVE(it->i == 0)
    primesieve_generate_prev_primes(it);
  it->i -= 1;
  return it->primes[it->i];
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
