///
/// @file PreSieve_x86_avx2.hpp
///
/// Copyright (C) 2026 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRESIEVE_X86_AVX2_HPP
#define PRESIEVE_X86_AVX2_HPP

#include <primesieve/cpu_arch_macros.hpp>
#include <primesieve/macros.hpp>

#include <immintrin.h>
#include <stdint.h>
#include <cstddef>

namespace {

#if defined(ENABLE_MULTIARCH_AVX2)
  __attribute__ ((target ("avx2")))
#endif
void presieve1_x86_avx2(const uint8_t* __restrict preSieved0,
                        const uint8_t* __restrict preSieved1,
                        const uint8_t* __restrict preSieved2,
                        const uint8_t* __restrict preSieved3,
                        uint8_t* __restrict sieve,
                        std::size_t bytes)
{
  std::size_t i = 0;
  std::size_t limit = bytes - bytes % sizeof(__m256i);

  // This loop executes only about 32 iterations on average.
  // Hence, unrolling this loop will most likely hurt
  // performance since it will cause more branch misses.
  NO_UNROLL_LOOP
  for (; i < limit; i += sizeof(__m256i))
  {
    _mm256_storeu_si256((__m256i*) &sieve[i],
      _mm256_and_si256(
        _mm256_and_si256(_mm256_loadu_si256((const __m256i*) &preSieved0[i]), _mm256_loadu_si256((const __m256i*) &preSieved1[i])),
        _mm256_and_si256(_mm256_loadu_si256((const __m256i*) &preSieved2[i]), _mm256_loadu_si256((const __m256i*) &preSieved3[i]))));
  }

  NO_UNROLL_LOOP
  NO_VECTORIZE_LOOP
  for (; i < bytes; i++)
    sieve[i] = preSieved0[i] & preSieved1[i] & preSieved2[i] & preSieved3[i];
}

#if defined(ENABLE_MULTIARCH_AVX2)
  __attribute__ ((target ("avx2")))
#endif
void presieve2_x86_avx2(const uint8_t* __restrict preSieved0,
                        const uint8_t* __restrict preSieved1,
                        const uint8_t* __restrict preSieved2,
                        const uint8_t* __restrict preSieved3,
                        uint8_t* __restrict sieve,
                        std::size_t bytes)
{
  std::size_t i = 0;
  std::size_t limit = bytes - bytes % sizeof(__m256i);

  // This loop executes only about 32 iterations on average.
  // Hence, unrolling this loop will most likely hurt
  // performance since it will cause more branch misses.
  NO_UNROLL_LOOP
  for (; i < limit; i += sizeof(__m256i))
  {
    _mm256_storeu_si256((__m256i*) &sieve[i],
      _mm256_and_si256(_mm256_loadu_si256((const __m256i*) &sieve[i]), _mm256_and_si256(
        _mm256_and_si256(_mm256_loadu_si256((const __m256i*) &preSieved0[i]), _mm256_loadu_si256((const __m256i*) &preSieved1[i])),
        _mm256_and_si256(_mm256_loadu_si256((const __m256i*) &preSieved2[i]), _mm256_loadu_si256((const __m256i*) &preSieved3[i])))));
  }

  NO_UNROLL_LOOP
  NO_VECTORIZE_LOOP
  for (; i < bytes; i++)
    sieve[i] &= preSieved0[i] & preSieved1[i] & preSieved2[i] & preSieved3[i];
}

} // namespace

#endif
