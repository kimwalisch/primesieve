///
/// @file PreSieve_x86_sse2.hpp
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
/// Copyright (C) 2022 @zielaj, https://github.com/zielaj
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRESIEVE_X86_SSE2_HPP
#define PRESIEVE_X86_SSE2_HPP

#include <emmintrin.h>
#include <stdint.h>
#include <cstddef>

namespace {

void presieve1_x86_sse2(const uint8_t* __restrict preSieved0,
                        const uint8_t* __restrict preSieved1,
                        const uint8_t* __restrict preSieved2,
                        const uint8_t* __restrict preSieved3,
                        uint8_t* __restrict sieve,
                        std::size_t bytes)
{
  std::size_t i = 0;
  std::size_t limit = bytes - bytes % sizeof(__m128i);

  for (; i < limit; i += sizeof(__m128i))
  {
    _mm_storeu_si128((__m128i*) &sieve[i],
      _mm_and_si128(
        _mm_and_si128(_mm_loadu_si128((const __m128i*) &preSieved0[i]), _mm_loadu_si128((const __m128i*) &preSieved1[i])),
        _mm_and_si128(_mm_loadu_si128((const __m128i*) &preSieved2[i]), _mm_loadu_si128((const __m128i*) &preSieved3[i]))));
  }

  for (; i < bytes; i++)
    sieve[i] = preSieved0[i] & preSieved1[i] & preSieved2[i] & preSieved3[i];
}

void presieve2_x86_sse2(const uint8_t* __restrict preSieved0,
                        const uint8_t* __restrict preSieved1,
                        const uint8_t* __restrict preSieved2,
                        const uint8_t* __restrict preSieved3,
                        uint8_t* __restrict sieve,
                        std::size_t bytes)
{
  std::size_t i = 0;
  std::size_t limit = bytes - bytes % sizeof(__m128i);

  for (; i < limit; i += sizeof(__m128i))
  {
    _mm_storeu_si128((__m128i*) &sieve[i],
      _mm_and_si128(_mm_loadu_si128((const __m128i*) &sieve[i]), _mm_and_si128(
        _mm_and_si128(_mm_loadu_si128((const __m128i*) &preSieved0[i]), _mm_loadu_si128((const __m128i*) &preSieved1[i])),
        _mm_and_si128(_mm_loadu_si128((const __m128i*) &preSieved2[i]), _mm_loadu_si128((const __m128i*) &preSieved3[i])))));
  }

  for (; i < bytes; i++)
    sieve[i] &= preSieved0[i] & preSieved1[i] & preSieved2[i] & preSieved3[i];
}

} // namespace

#endif
