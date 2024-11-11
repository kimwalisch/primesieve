///
/// @file   PreSieve.cpp
/// @brief  Pre-sieve multiples of small primes <= 163 to speed up the
///         sieve of Eratosthenes. We use 16 static lookup tables from
///         which the multiples of small primes have been removed
///         upfront. Each preSieve lookup table is assigned different
///         primes used for pre-sieving:
///
///         preSieveTable[0]  = {  7, 23, 37 }
///         preSieveTable[1]  = { 11, 19, 31 }
///         preSieveTable[2]  = { 13, 17, 29 }
///         preSieveTable[3]  = { 41, 163 }
///         preSieveTable[4]  = { 43, 157 }
///         preSieveTable[5]  = { 47, 151 }
///         preSieveTable[6]  = { 53, 149 }
///         preSieveTable[7]  = { 59, 139 }
///         preSieveTable[8]  = { 61, 137 }
///         preSieveTable[9]  = { 67, 131 }
///         preSieveTable[10] = { 71, 127 }
///         preSieveTable[11] = { 73, 113 }
///         preSieveTable[12] = { 79, 109 }
///         preSieveTable[13] = { 83, 107 }
///         preSieveTable[14] = { 89, 103 }
///         preSieveTable[15] = { 97, 101 }
///
///         The total size of these 16 preSieveTables is 123
///         kilobytes. Whilst sieving, we perform a bitwise AND of all
///         preSieveTables and store the result in the sieve array.
///         Pre-sieving provides a speedup of up to 30% when sieving
///         the primes < 10^10 using primesieve.
///
/// Copyright (C) 2024 Kim Walisch, <kim.walisch@gmail.com>
/// Copyright (C) 2022 @zielaj, https://github.com/zielaj
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/PreSieve.hpp>
#include <primesieve/PreSieveTables.hpp>
#include <primesieve/Vector.hpp>
#include <primesieve/macros.hpp>

#include <stdint.h>
#include <algorithm>
#include <cstddef>

#if defined(__ARM_FEATURE_SVE) && \
    __has_include(<arm_sve.h>)
  #include <arm_sve.h>
  #define ENABLE_ARM_SVE

#elif defined(__AVX512F__) && \
      defined(__AVX512BW__) && \
      __has_include(<immintrin.h>)
  #include <immintrin.h>
  #define ENABLE_AVX512_BW

#elif defined(ENABLE_MULTIARCH_AVX512_BW) && \
      __has_include(<immintrin.h>)
  #include <primesieve/cpu_supports_avx512_bw.hpp>
  #include <immintrin.h>
  #define ENABLE_DEFAULT
#else
  #define ENABLE_DEFAULT
#endif

#if defined(ENABLE_DEFAULT)
  /// All x64 CPUs support the SSE2 vector instruction set
  #if defined(__SSE2__) && \
      __has_include(<emmintrin.h>)
    #include <emmintrin.h>
    #define HAS_SSE2
  #endif
  // All ARM64 CPUs support the NEON vector instruction set
  #if (defined(__ARM_NEON) || defined(__aarch64__)) && \
      __has_include(<arm_neon.h>)
    #include <arm_neon.h>
    #define HAS_ARM_NEON
  #endif
#endif

namespace {

#if defined(ENABLE_ARM_SVE)

void AND_PreSieveTables_arm_sve(const uint8_t* __restrict preSieved0,
                                const uint8_t* __restrict preSieved1,
                                const uint8_t* __restrict preSieved2,
                                const uint8_t* __restrict preSieved3,
                                uint8_t* __restrict sieve,
                                std::size_t bytes)
{
  for (std::size_t i = 0; i < bytes; i += svcntb())
  {
    svbool_t pg = svwhilelt_b8(i, bytes);

    svst1_u8(pg, &sieve[i],
      svand_u8_x(svptrue_b64(),
        svand_u8_x(svptrue_b64(), svld1_u8(pg, &preSieved0[i]), svld1_u8(pg, &preSieved1[i])),
        svand_u8_x(svptrue_b64(), svld1_u8(pg, &preSieved2[i]), svld1_u8(pg, &preSieved3[i]))));
  }
}

void AND_PreSieveTables_Sieve_arm_sve(const uint8_t* __restrict preSieved0,
                                      const uint8_t* __restrict preSieved1,
                                      const uint8_t* __restrict preSieved2,
                                      const uint8_t* __restrict preSieved3,
                                      uint8_t* __restrict sieve,
                                      std::size_t bytes)
{
  for (std::size_t i = 0; i < bytes; i += svcntb())
  {
    svbool_t pg = svwhilelt_b8(i, bytes);

    svst1_u8(pg, &sieve[i],
      svand_u8_x(svptrue_b64(), svld1_u8(pg, &sieve[i]), svand_u8_x(svptrue_b64(),
        svand_u8_x(svptrue_b64(), svld1_u8(pg, &preSieved0[i]), svld1_u8(pg, &preSieved1[i])),
        svand_u8_x(svptrue_b64(), svld1_u8(pg, &preSieved2[i]), svld1_u8(pg, &preSieved3[i])))));
  }
}

#elif defined(ENABLE_AVX512_BW) || \
      defined(ENABLE_MULTIARCH_AVX512_BW)

#if defined(ENABLE_MULTIARCH_AVX512_BW)
  __attribute__ ((target ("avx512f,avx512bw")))
#endif
void AND_PreSieveTables_avx512(const uint8_t* __restrict preSieved0,
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
void AND_PreSieveTables_Sieve_avx512(const uint8_t* __restrict preSieved0,
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

#endif

/// This section contains portable SIMD algorithms that don't need
/// any runtime CPU support checks.
#if defined(ENABLE_DEFAULT)
#if defined(HAS_SSE2)

void AND_PreSieveTables_default(const uint8_t* __restrict preSieved0,
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

void AND_PreSieveTables_Sieve_default(const uint8_t* __restrict preSieved0,
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

#elif defined(HAS_ARM_NEON)

void AND_PreSieveTables_default(const uint8_t* __restrict preSieved0,
                                const uint8_t* __restrict preSieved1,
                                const uint8_t* __restrict preSieved2,
                                const uint8_t* __restrict preSieved3,
                                uint8_t* __restrict sieve,
                                std::size_t bytes)
{
  std::size_t i = 0;
  std::size_t limit = bytes - bytes % sizeof(uint8x16_t);

  for (; i < limit; i += sizeof(uint8x16_t))
  {
    vst1q_u8(&sieve[i],
        vandq_u8(
            vandq_u8(vld1q_u8(&preSieved0[i]), vld1q_u8(&preSieved1[i])),
            vandq_u8(vld1q_u8(&preSieved2[i]), vld1q_u8(&preSieved3[i]))));
  }

  for (; i < bytes; i++)
    sieve[i] = preSieved0[i] & preSieved1[i] & preSieved2[i] & preSieved3[i];
}

void AND_PreSieveTables_Sieve_default(const uint8_t* __restrict preSieved0,
                                      const uint8_t* __restrict preSieved1,
                                      const uint8_t* __restrict preSieved2,
                                      const uint8_t* __restrict preSieved3,
                                      uint8_t* __restrict sieve,
                                      std::size_t bytes)
{
  std::size_t i = 0;
  std::size_t limit = bytes - bytes % sizeof(uint8x16_t);

  for (; i < limit; i += sizeof(uint8x16_t))
  {
    vst1q_u8(&sieve[i],
        vandq_u8(vld1q_u8(&sieve[i]), vandq_u8(
            vandq_u8(vld1q_u8(&preSieved0[i]), vld1q_u8(&preSieved1[i])),
            vandq_u8(vld1q_u8(&preSieved2[i]), vld1q_u8(&preSieved3[i])))));
  }

  for (; i < bytes; i++)
    sieve[i] &= preSieved0[i] & preSieved1[i] & preSieved2[i] & preSieved3[i];
}

#else

void AND_PreSieveTables_default(const uint8_t* __restrict preSieved0,
                                const uint8_t* __restrict preSieved1,
                                const uint8_t* __restrict preSieved2,
                                const uint8_t* __restrict preSieved3,
                                uint8_t* __restrict sieve,
                                std::size_t bytes)
{
  for (std::size_t i = 0; i < bytes; i++)
    sieve[i] = preSieved0[i] & preSieved1[i] & preSieved2[i] & preSieved3[i];
}

void AND_PreSieveTables_Sieve_default(const uint8_t* __restrict preSieved0,
                                      const uint8_t* __restrict preSieved1,
                                      const uint8_t* __restrict preSieved2,
                                      const uint8_t* __restrict preSieved3,
                                      uint8_t* __restrict sieve,
                                      std::size_t bytes)
{
  for (std::size_t i = 0; i < bytes; i++)
    sieve[i] &= preSieved0[i] & preSieved1[i] & preSieved2[i] & preSieved3[i];
}

#endif
#endif

void AND_PreSieveTables(const uint8_t* __restrict preSieved0,
                        const uint8_t* __restrict preSieved1,
                        const uint8_t* __restrict preSieved2,
                        const uint8_t* __restrict preSieved3,
                        uint8_t* __restrict sieve,
                        std::size_t bytes)
{
#if defined(ENABLE_ARM_SVE)
  AND_PreSieveTables_arm_sve(preSieved0, preSieved1, preSieved2, preSieved3, sieve, bytes);
#elif defined(ENABLE_AVX512_BW)
  AND_PreSieveTables_avx512(preSieved0, preSieved1, preSieved2, preSieved3, sieve, bytes);
#elif defined(ENABLE_MULTIARCH_AVX512_BW)
  if (cpu_supports_avx512_bw)
    AND_PreSieveTables_avx512(preSieved0, preSieved1, preSieved2, preSieved3, sieve, bytes);
  else
    AND_PreSieveTables_default(preSieved0, preSieved1, preSieved2, preSieved3, sieve, bytes);
#else
  AND_PreSieveTables_default(preSieved0, preSieved1, preSieved2, preSieved3, sieve, bytes);
#endif
}

void AND_PreSieveTables_Sieve(const uint8_t* __restrict preSieved0,
                              const uint8_t* __restrict preSieved1,
                              const uint8_t* __restrict preSieved2,
                              const uint8_t* __restrict preSieved3,
                              uint8_t* __restrict sieve,
                              std::size_t bytes)
{
#if defined(ENABLE_ARM_SVE)
  AND_PreSieveTables_Sieve_arm_sve(preSieved0, preSieved1, preSieved2, preSieved3, sieve, bytes);
#elif defined(ENABLE_AVX512_BW)
  AND_PreSieveTables_Sieve_avx512(preSieved0, preSieved1, preSieved2, preSieved3, sieve, bytes);
#elif defined(ENABLE_MULTIARCH_AVX512_BW)
  if (cpu_supports_avx512_bw)
    AND_PreSieveTables_Sieve_avx512(preSieved0, preSieved1, preSieved2, preSieved3, sieve, bytes);
  else
    AND_PreSieveTables_Sieve_default(preSieved0, preSieved1, preSieved2, preSieved3, sieve, bytes);
#else
  AND_PreSieveTables_Sieve_default(preSieved0, preSieved1, preSieved2, preSieved3, sieve, bytes);
#endif
}

} // namespace

namespace primesieve {

void PreSieve::preSieve(Vector<uint8_t>& sieve, uint64_t segmentLow)
{
  uint64_t offset = 0;
  Array<uint64_t, 4> pos;

  pos[0] = (segmentLow % (preSieveTables[0].size() * 30)) / 30;
  pos[1] = (segmentLow % (preSieveTables[1].size() * 30)) / 30;
  pos[2] = (segmentLow % (preSieveTables[2].size() * 30)) / 30;
  pos[3] = (segmentLow % (preSieveTables[3].size() * 30)) / 30;

  while (offset < sieve.size())
  {
    uint64_t bytesToCopy = sieve.size() - offset;

    bytesToCopy = std::min(bytesToCopy, uint64_t(preSieveTables[0].size() - pos[0]));
    bytesToCopy = std::min(bytesToCopy, uint64_t(preSieveTables[1].size() - pos[1]));
    bytesToCopy = std::min(bytesToCopy, uint64_t(preSieveTables[2].size() - pos[2]));
    bytesToCopy = std::min(bytesToCopy, uint64_t(preSieveTables[3].size() - pos[3]));

    AND_PreSieveTables(&*(preSieveTables[0].begin() + pos[0]),
                       &*(preSieveTables[1].begin() + pos[1]),
                       &*(preSieveTables[2].begin() + pos[2]),
                       &*(preSieveTables[3].begin() + pos[3]),
                       &sieve[offset],
                       bytesToCopy);

    offset += bytesToCopy;

    pos[0] = (pos[0] + bytesToCopy) * (pos[0] < preSieveTables[0].size());
    pos[1] = (pos[1] + bytesToCopy) * (pos[1] < preSieveTables[1].size());
    pos[2] = (pos[2] + bytesToCopy) * (pos[2] < preSieveTables[2].size());
    pos[3] = (pos[3] + bytesToCopy) * (pos[3] < preSieveTables[3].size());
  }

  for (std::size_t i = pos.size(); i < preSieveTables.size(); i += 4)
  {
    offset = 0;

    pos[0] = (segmentLow % (preSieveTables[i+0].size() * 30)) / 30;
    pos[1] = (segmentLow % (preSieveTables[i+1].size() * 30)) / 30;
    pos[2] = (segmentLow % (preSieveTables[i+2].size() * 30)) / 30;
    pos[3] = (segmentLow % (preSieveTables[i+3].size() * 30)) / 30;

    while (offset < sieve.size())
    {
      uint64_t bytesToCopy = sieve.size() - offset;

      bytesToCopy = std::min(bytesToCopy, uint64_t(preSieveTables[i+0].size() - pos[0]));
      bytesToCopy = std::min(bytesToCopy, uint64_t(preSieveTables[i+1].size() - pos[1]));
      bytesToCopy = std::min(bytesToCopy, uint64_t(preSieveTables[i+2].size() - pos[2]));
      bytesToCopy = std::min(bytesToCopy, uint64_t(preSieveTables[i+3].size() - pos[3]));

      AND_PreSieveTables_Sieve(&*(preSieveTables[i+0].begin() + pos[0]),
                               &*(preSieveTables[i+1].begin() + pos[1]),
                               &*(preSieveTables[i+2].begin() + pos[2]),
                               &*(preSieveTables[i+3].begin() + pos[3]),
                               &sieve[offset],
                               bytesToCopy);

      offset += bytesToCopy;

      pos[0] = (pos[0] + bytesToCopy) * (pos[0] < preSieveTables[i+0].size());
      pos[1] = (pos[1] + bytesToCopy) * (pos[1] < preSieveTables[i+1].size());
      pos[2] = (pos[2] + bytesToCopy) * (pos[2] < preSieveTables[i+2].size());
      pos[3] = (pos[3] + bytesToCopy) * (pos[3] < preSieveTables[i+3].size());
    }
  }

  // Pre-sieving removes the primes <= 163. We
  // have to undo that work and reset these bits
  // to 1 (but 49 = 7 * 7 is not a prime).
  if (segmentLow <= getMaxPrime())
  {
    uint64_t i = segmentLow / 30;
    uint8_t* sieveArray = sieve.data();
    Array<uint8_t, 8> primeBits = { 0xff, 0xef, 0x77, 0x3f, 0xdb, 0xed, 0x9e, 0xfc };

    ASSERT(sieve.capacity() >= primeBits.size());
    for (std::size_t j = 0; i + j < primeBits.size(); j++)
      sieveArray[j] = primeBits[i + j];
  }
}

} // namespace
