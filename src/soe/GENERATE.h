//
// Copyright (c) 2012 Kim Walisch, <kim.walisch@gmail.com>.
// All rights reserved.
//
// This file is part of primesieve.
// Homepage: http://primesieve.googlecode.com
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//   * Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above
//     copyright notice, this list of conditions and the following
//     disclaimer in the documentation and/or other materials provided
//     with the distribution.
//   * Neither the name of the author nor the names of its
//     contributors may be used to endorse or promote products derived
//     from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef GENERATE_PRIMESIEVE_H
#define GENERATE_PRIMESIEVE_H

#include "config.h"
#include "SieveOfEratosthenes-inline.h"
#include "endian_safe_cast.h"

#include <stdint.h>

/// Reconstruct prime numbers from 1 bits of the sieve array and call
/// a callback function for each prime.
///
#define GENERATE_PRIMES(callback, type)                       \
{                                                             \
  for (uint_t i = 0; i < sieveSize; i += 8) {                 \
    uint64_t bits = endian_safe_cast<uint64_t>(&sieve[i]);    \
    while (bits != 0)                                         \
      callback (static_cast<type>( getNextPrime(&bits, i) )); \
  }                                                           \
}

/// Reconstruct twin primes from 11 bit patterns of the sieve array.
/// For each twin prime pair (p1, p2) the first prime p1 is called
/// back i.e. callback( p1 ).
///
#define GENERATE_TWINS(callback, type)                        \
{                                                             \
  for (uint_t i = 0; i < sieveSize; i += 8) {                 \
    uint64_t bits = endian_safe_cast<uint64_t>(&sieve[i]);    \
    bits &= (bits >> 1) & 0x4A4A4A4A4A4A4A4Aull;              \
    while (bits != 0)                                         \
      callback (static_cast<type>( getNextPrime(&bits, i) )); \
  }                                                           \
}

/// Reconstruct prime triplets from 111 bit patterns of the sieve
/// array. For each prime triplet (p1, p2, p3) the first prime p1 is
/// called back i.e. callback( p1 ).
///
#define GENERATE_TRIPLETS(callback, type)                     \
{                                                             \
  for (uint_t i = 0; i < sieveSize; i += 8) {                 \
    uint64_t bits = endian_safe_cast<uint64_t>(&sieve[i]);    \
    bits &= (bits >> 1);                                      \
    bits &= (bits >> 1) & 0x0F0F0F0F0F0F0F0Full;              \
    while (bits != 0)                                         \
      callback (static_cast<type>( getNextPrime(&bits, i) )); \
  }                                                           \
}

#endif
