///
/// @file PreSieve_x86_avx512.hpp
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
/// Copyright (C) 2022 @zielaj, https://github.com/zielaj
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRESIEVE_X86_AVX512_HPP
#define PRESIEVE_X86_AVX512_HPP

#include <immintrin.h>
#include <stdint.h>
#include <cstddef>

namespace {

#if defined(ENABLE_MULTIARCH_AVX512_BW)
  __attribute__ ((target ("avx512f,avx512bw")))
#endif
void presieve1_x86_avx512(const uint8_t* __restrict preSieved0,
                          const uint8_t* __restrict preSieved1,
                          const uint8_t* __restrict preSieved2,
                          const uint8_t* __restrict preSieved3,
                          uint8_t* __restrict sieve,
                          std::size_t bytes)
{
  std::size_t i = 0;

  for (; i + 64 <= bytes; i += sizeof(__m512i))
  {
    _mm512_storeu_epi8((__m512i*) &sieve[i],
      _mm512_and_si512(
        _mm512_and_si512(_mm512_loadu_epi8((const __m512i*) &preSieved0[i]), _mm512_loadu_epi8((const __m512i*) &preSieved1[i])),
        _mm512_and_si512(_mm512_loadu_epi8((const __m512i*) &preSieved2[i]), _mm512_loadu_epi8((const __m512i*) &preSieved3[i]))));
  }

  if (i < bytes)
  {
    __mmask64 mask = 0xffffffffffffffffull >> (i + 64 - bytes);

    _mm512_mask_storeu_epi8((__m512i*) &sieve[i], mask,
      _mm512_and_si512(
        _mm512_and_si512(_mm512_maskz_loadu_epi8(mask, (const __m512i*) &preSieved0[i]), _mm512_maskz_loadu_epi8(mask, (const __m512i*) &preSieved1[i])),
        _mm512_and_si512(_mm512_maskz_loadu_epi8(mask, (const __m512i*) &preSieved2[i]), _mm512_maskz_loadu_epi8(mask, (const __m512i*) &preSieved3[i]))));
  }
}

#if defined(ENABLE_MULTIARCH_AVX512_BW)
  __attribute__ ((target ("avx512f,avx512bw")))
#endif
void presieve2_x86_avx512(const uint8_t* __restrict preSieved0,
                          const uint8_t* __restrict preSieved1,
                          const uint8_t* __restrict preSieved2,
                          const uint8_t* __restrict preSieved3,
                          uint8_t* __restrict sieve,
                          std::size_t bytes)
{
  std::size_t i = 0;

  for (; i + 64 <= bytes; i += sizeof(__m512i))
  {
    _mm512_storeu_epi8((__m512i*) &sieve[i],
      _mm512_and_si512(_mm512_loadu_epi8((const __m512i*) &sieve[i]), _mm512_and_si512(
        _mm512_and_si512(_mm512_loadu_epi8((const __m512i*) &preSieved0[i]), _mm512_loadu_epi8((const __m512i*) &preSieved1[i])),
        _mm512_and_si512(_mm512_loadu_epi8((const __m512i*) &preSieved2[i]), _mm512_loadu_epi8((const __m512i*) &preSieved3[i])))));
  }

  if (i < bytes)
  {
    __mmask64 mask = 0xffffffffffffffffull >> (i + 64 - bytes);

    _mm512_mask_storeu_epi8((__m512i*) &sieve[i], mask,
      _mm512_and_si512(_mm512_maskz_loadu_epi8(mask, (const __m512i*) &sieve[i]), _mm512_and_si512(
        _mm512_and_si512(_mm512_maskz_loadu_epi8(mask, (const __m512i*) &preSieved0[i]), _mm512_maskz_loadu_epi8(mask, (const __m512i*) &preSieved1[i])),
        _mm512_and_si512(_mm512_maskz_loadu_epi8(mask, (const __m512i*) &preSieved2[i]), _mm512_maskz_loadu_epi8(mask, (const __m512i*) &preSieved3[i])))));
  }
}

} // namespace

#endif
