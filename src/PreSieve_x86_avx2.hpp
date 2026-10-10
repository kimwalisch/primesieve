///
/// @file PreSieve_x86_avx2.hpp
///
/// Copyright (C) 2026 Kim Walisch, <kim.walisch@gmail.com>
/// Copyright (C) 2022 @zielaj, https://github.com/zielaj
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

#if defined(ENABLE_PRESIEVE_AVX2_STATS)
  #include <inttypes.h>
  #include <atomic>
  #include <cstdio>
#endif

namespace {

#if defined(ENABLE_PRESIEVE_AVX2_STATS)
/// Enable with -DENABLE_PRESIEVE_AVX2_STATS to print statistics at exit.
/// Atomic counters allow collecting statistics with multiple threads.
/// Instrumented timings should not be used for performance comparisons.
struct PreSieve1Avx2Stats
{
  std::atomic<uint64_t> calls{0};
  std::atomic<uint64_t> emptyCalls{0};
  std::atomic<uint64_t> avx2Iterations{0};
  std::atomic<uint64_t> tailIterations{0};

  ~PreSieve1Avx2Stats()
  {
    uint64_t count = calls.load(std::memory_order_relaxed);
    if (count == 0)
      return;

    uint64_t empty = emptyCalls.load(std::memory_order_relaxed);
    uint64_t avx2 = avx2Iterations.load(std::memory_order_relaxed);
    uint64_t tail = tailIterations.load(std::memory_order_relaxed);
    double nonEmpty = (count > empty) ? double(count - empty) : 1;
    double avx2Bytes = double(avx2) * sizeof(__m256i);

    std::fprintf(stderr,
        "presieve1_x86_avx2 statistics:\n"
        "  Calls: %" PRIu64 " (%" PRIu64 " empty)\n"
        "  AVX2 iterations: %" PRIu64 " total, %.3f per call, %.3f per non-empty call\n"
        "  AVX2 bytes: %.3f per call, %.3f per non-empty call\n"
        "  Scalar tail iterations: %" PRIu64 " total, %.3f per call, %.3f per non-empty call\n"
        "  Input bytes: %.3f per call, %.3f per non-empty call\n",
        count, empty,
        avx2, double(avx2) / count, double(avx2) / nonEmpty,
        avx2Bytes / count, avx2Bytes / nonEmpty,
        tail, double(tail) / count, double(tail) / nonEmpty,
        (avx2Bytes + tail) / count, (avx2Bytes + tail) / nonEmpty);
  }
};

PreSieve1Avx2Stats preSieve1Avx2Stats;
#endif

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

  for (; i < limit; i += sizeof(__m256i))
  {
    _mm256_storeu_si256((__m256i*) &sieve[i],
      _mm256_and_si256(
        _mm256_and_si256(_mm256_loadu_si256((const __m256i*) &preSieved0[i]), _mm256_loadu_si256((const __m256i*) &preSieved1[i])),
        _mm256_and_si256(_mm256_loadu_si256((const __m256i*) &preSieved2[i]), _mm256_loadu_si256((const __m256i*) &preSieved3[i]))));
  }

  #if defined(ENABLE_PRESIEVE_AVX2_STATS)
    // Update counters once per call, outside the AVX2 and scalar loops.
    preSieve1Avx2Stats.calls.fetch_add(1, std::memory_order_relaxed);
    preSieve1Avx2Stats.avx2Iterations.fetch_add(i / sizeof(__m256i), std::memory_order_relaxed);
    preSieve1Avx2Stats.tailIterations.fetch_add(bytes - i, std::memory_order_relaxed);
    if (bytes == 0)
      preSieve1Avx2Stats.emptyCalls.fetch_add(1, std::memory_order_relaxed);
  #endif

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
