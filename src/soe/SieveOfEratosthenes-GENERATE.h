///
/// @file   SieveOfEratosthenes-GENERATE.h
/// @brief  Macros to reconstruct primes and prime k-tuplets from 1
///         bits of the sieve array.
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef GENERATE_PRIMESIEVE_H
#define GENERATE_PRIMESIEVE_H

#include "config.h"
#include "SieveOfEratosthenes-inline.h"
#include "endiansafe_cast.h"

#include <stdint.h>

/// Reconstruct prime numbers from 1 bits of the sieve array and
/// call a callback function for each prime.
///
#define GENERATE_PRIMES(callback, type)                       \
{                                                             \
  for (uint_t i = 0; i < sieveSize; i += 8) {                 \
    uint64_t bits = endiansafe_cast<uint64_t>(&sieve[i]);     \
    while (bits != 0)                                         \
      callback (static_cast<type>( getNextPrime(&bits, i) )); \
  }                                                           \
}

/// Reconstruct twin primes from 11 bit patterns within the sieve
/// array. For each twin prime pair (p1, p2) the first prime p1
/// is called back i.e. callback(p1).
///
#define GENERATE_TWINS(callback, type)                        \
{                                                             \
  for (uint_t i = 0; i < sieveSize; i += 8) {                 \
    uint64_t bits = endiansafe_cast<uint64_t>(&sieve[i]);     \
    bits &= (bits >> 1) & 0x4A4A4A4A4A4A4A4Aull;              \
    while (bits != 0)                                         \
      callback (static_cast<type>( getNextPrime(&bits, i) )); \
  }                                                           \
}

/// Reconstruct prime triplets from 111 bit patterns within the sieve
/// array. For each prime triplet (p1, p2, p3) the first prime p1
/// is called back i.e. callback(p1).
///
#define GENERATE_TRIPLETS(callback, type)                     \
{                                                             \
  for (uint_t i = 0; i < sieveSize; i += 8) {                 \
    uint64_t bits = endiansafe_cast<uint64_t>(&sieve[i]);     \
    bits &= (bits >> 1);                                      \
    bits &= (bits >> 1) & 0x0F0F0F0F0F0F0F0Full;              \
    while (bits != 0)                                         \
      callback (static_cast<type>( getNextPrime(&bits, i) )); \
  }                                                           \
}

#endif
