///
/// @file PreSieve_x86_avx2.hpp
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRESIEVE_X86_AVX2_HPP
#define PRESIEVE_X86_AVX2_HPP

#include <immintrin.h>
#include <stdint.h>
#include <cstddef>

namespace {

/// Pre-sieve multiples of small primes <= 19 using AVX2 (256-bit SIMD).
/// This algorithm processes 32 bytes per iteration using AVX2
/// vector instructions. AVX2 is available on most modern CPUs:
/// - Intel Haswell (2013+) and later
/// - AMD Zen (2017+) and later
/// This provides a good middle ground between SSE2 (16 bytes)
/// and AVX512 (64 bytes), benefiting ~95% of modern x86 CPUs.
///
void presieve1_x86_avx2(const uint8_t* __restrict preSieved0,
                        const uint8_t* __restrict preSieved1,
                        const uint8_t* __restrict preSieved2,
                        const uint8_t* __restrict preSieved3,
                        uint8_t* __restrict sieve,
                        std::size_t bytes)
{
  std::size_t i = 0;

  // Process 32 bytes per iteration using AVX2
  for (; i + 32 <= bytes; i += 32)
  {
    __m256i v0 = _mm256_loadu_si256((const __m256i*) &preSieved0[i]);
    __m256i v1 = _mm256_loadu_si256((const __m256i*) &preSieved1[i]);
    __m256i v2 = _mm256_loadu_si256((const __m256i*) &preSieved2[i]);
    __m256i v3 = _mm256_loadu_si256((const __m256i*) &preSieved3[i]);

    __m256i combined = _mm256_and_si256(v0, v1);
    combined = _mm256_and_si256(combined, v2);
    combined = _mm256_and_si256(combined, v3);

    _mm256_storeu_si256((__m256i*) &sieve[i], combined);
  }

  // Process remaining bytes
  for (; i < bytes; i++)
    sieve[i] = preSieved0[i] & preSieved1[i] & preSieved2[i] & preSieved3[i];
}

/// Pre-sieve multiples of small primes <= 163 using AVX2.
/// Uses 16 pre-sieve tables (4 sets of 4 tables).
/// Processing 32 bytes per iteration.
///
void presieve2_x86_avx2(const uint8_t* __restrict preSieved0,
                        const uint8_t* __restrict preSieved1,
                        const uint8_t* __restrict preSieved2,
                        const uint8_t* __restrict preSieved3,
                        uint8_t* __restrict sieve,
                        std::size_t bytes)
{
  std::size_t i = 0;

  // Process 32 bytes per iteration using AVX2
  for (; i + 32 <= bytes; i += 32)
  {
    __m256i v0 = _mm256_loadu_si256((const __m256i*) &preSieved0[i]);
    __m256i v1 = _mm256_loadu_si256((const __m256i*) &preSieved1[i]);
    __m256i v2 = _mm256_loadu_si256((const __m256i*) &preSieved2[i]);
    __m256i v3 = _mm256_loadu_si256((const __m256i*) &preSieved3[i]);

    __m256i combined = _mm256_and_si256(v0, v1);
    combined = _mm256_and_si256(combined, v2);
    combined = _mm256_and_si256(combined, v3);

    _mm256_storeu_si256((__m256i*) &sieve[i], combined);
  }

  // Process remaining bytes
  for (; i < bytes; i++)
    sieve[i] &= preSieved0[i] & preSieved1[i] & preSieved2[i] & preSieved3[i];
}

} // namespace

#endif