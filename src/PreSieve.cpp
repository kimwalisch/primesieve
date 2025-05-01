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
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
/// Copyright (C) 2022 @zielaj, https://github.com/zielaj
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include "PreSieve.hpp"
#include "PreSieveTables.hpp"

#include <primesieve/Vector.hpp>
#include <primesieve/macros.hpp>

#include <stdint.h>
#include <algorithm>
#include <cstddef>

#if defined(__ARM_FEATURE_SVE) && \
    __has_include(<arm_sve.h>)
  #include "PreSieve_arm_sve.hpp"
  #define presieve1_default presieve1_arm_sve
  #define presieve2_default presieve2_arm_sve

#elif defined(__AVX512F__) && \
      defined(__AVX512BW__) && \
      __has_include(<immintrin.h>)
  #include "PreSieve_x86_avx512.hpp"
  #define presieve1_default presieve1_x86_avx512
  #define presieve2_default presieve2_x86_avx512

#elif defined(ENABLE_MULTIARCH_ARM_SVE)
  #include <primesieve/cpu_supports_arm_sve.hpp>
  #include "PreSieve_arm_sve.hpp"

#elif defined(ENABLE_MULTIARCH_AVX512_BW)
  #include <primesieve/cpu_supports_avx512_bw.hpp>
  #include "PreSieve_x86_avx512.hpp"
#endif

// Portable algorithms that run on any CPU
#if !defined(presieve1_default) || \
    !defined(presieve2_default)

#if defined(__SSE2__) && \
    __has_include(<emmintrin.h>)
  #include "PreSieve_x86_sse2.hpp"
  #define presieve1_default presieve1_x86_sse2
  #define presieve2_default presieve2_x86_sse2

#elif (defined(__ARM_NEON) || defined(__aarch64__)) && \
      __has_include(<arm_neon.h>)
  #include "PreSieve_arm_neon.hpp"
  #define presieve1_default presieve1_arm_neon
  #define presieve2_default presieve2_arm_neon
#else
  #include "PreSieve_default.hpp"
#endif

#endif

namespace {

ALWAYS_INLINE void presieve1(const uint8_t* __restrict preSieved0,
                             const uint8_t* __restrict preSieved1,
                             const uint8_t* __restrict preSieved2,
                             const uint8_t* __restrict preSieved3,
                             uint8_t* __restrict sieve,
                             std::size_t bytes)
{
#if defined(ENABLE_MULTIARCH_AVX512_BW)
  if (cpu_supports_avx512_bw)
    presieve1_x86_avx512(preSieved0, preSieved1, preSieved2, preSieved3, sieve, bytes);
  else
    presieve1_default(preSieved0, preSieved1, preSieved2, preSieved3, sieve, bytes);

#elif defined(ENABLE_MULTIARCH_ARM_SVE)
  if (cpu_supports_sve)
    presieve1_arm_sve(preSieved0, preSieved1, preSieved2, preSieved3, sieve, bytes);
  else
    presieve1_default(preSieved0, preSieved1, preSieved2, preSieved3, sieve, bytes);

#else
  presieve1_default(preSieved0, preSieved1, preSieved2, preSieved3, sieve, bytes);
#endif
}

ALWAYS_INLINE void presieve2(const uint8_t* __restrict preSieved0,
                             const uint8_t* __restrict preSieved1,
                             const uint8_t* __restrict preSieved2,
                             const uint8_t* __restrict preSieved3,
                             uint8_t* __restrict sieve,
                             std::size_t bytes)
{
#if defined(ENABLE_MULTIARCH_AVX512_BW)
  if (cpu_supports_avx512_bw)
    presieve2_x86_avx512(preSieved0, preSieved1, preSieved2, preSieved3, sieve, bytes);
  else
    presieve2_default(preSieved0, preSieved1, preSieved2, preSieved3, sieve, bytes);

#elif defined(ENABLE_MULTIARCH_ARM_SVE)
  if (cpu_supports_sve)
    presieve2_arm_sve(preSieved0, preSieved1, preSieved2, preSieved3, sieve, bytes);
  else
    presieve2_default(preSieved0, preSieved1, preSieved2, preSieved3, sieve, bytes);

#else
  presieve2_default(preSieved0, preSieved1, preSieved2, preSieved3, sieve, bytes);
#endif
}

} // namespace

namespace primesieve {

void PreSieve::preSieve(Vector<uint8_t>& sieve, uint64_t segmentLow)
{
  uint64_t offset = 0;
  uint64_t pos0, pos1, pos2, pos3;

  pos0 = (segmentLow % (preSieveTables[0].size() * 30)) / 30;
  pos1 = (segmentLow % (preSieveTables[1].size() * 30)) / 30;
  pos2 = (segmentLow % (preSieveTables[2].size() * 30)) / 30;
  pos3 = (segmentLow % (preSieveTables[3].size() * 30)) / 30;

  // PreSieve algo 1
  while (offset < sieve.size())
  {
    uint64_t bytesToCopy = sieve.size() - offset;

    bytesToCopy = std::min(bytesToCopy, uint64_t(preSieveTables[0].size() - pos0));
    bytesToCopy = std::min(bytesToCopy, uint64_t(preSieveTables[1].size() - pos1));
    bytesToCopy = std::min(bytesToCopy, uint64_t(preSieveTables[2].size() - pos2));
    bytesToCopy = std::min(bytesToCopy, uint64_t(preSieveTables[3].size() - pos3));

    presieve1(&*(preSieveTables[0].begin() + pos0),
              &*(preSieveTables[1].begin() + pos1),
              &*(preSieveTables[2].begin() + pos2),
              &*(preSieveTables[3].begin() + pos3),
              &sieve[offset],
              bytesToCopy);

    offset += bytesToCopy;

    pos0 = (pos0 + bytesToCopy) * (pos0 < preSieveTables[0].size());
    pos1 = (pos1 + bytesToCopy) * (pos1 < preSieveTables[1].size());
    pos2 = (pos2 + bytesToCopy) * (pos2 < preSieveTables[2].size());
    pos3 = (pos3 + bytesToCopy) * (pos3 < preSieveTables[3].size());
  }

  // PreSieve algo 2
  for (std::size_t i = 4; i < preSieveTables.size(); i += 4)
  {
    offset = 0;

    pos0 = (segmentLow % (preSieveTables[i+0].size() * 30)) / 30;
    pos1 = (segmentLow % (preSieveTables[i+1].size() * 30)) / 30;
    pos2 = (segmentLow % (preSieveTables[i+2].size() * 30)) / 30;
    pos3 = (segmentLow % (preSieveTables[i+3].size() * 30)) / 30;

    while (offset < sieve.size())
    {
      uint64_t bytesToCopy = sieve.size() - offset;

      bytesToCopy = std::min(bytesToCopy, uint64_t(preSieveTables[i+0].size() - pos0));
      bytesToCopy = std::min(bytesToCopy, uint64_t(preSieveTables[i+1].size() - pos1));
      bytesToCopy = std::min(bytesToCopy, uint64_t(preSieveTables[i+2].size() - pos2));
      bytesToCopy = std::min(bytesToCopy, uint64_t(preSieveTables[i+3].size() - pos3));

      presieve2(&*(preSieveTables[i+0].begin() + pos0),
                &*(preSieveTables[i+1].begin() + pos1),
                &*(preSieveTables[i+2].begin() + pos2),
                &*(preSieveTables[i+3].begin() + pos3),
                &sieve[offset],
                bytesToCopy);

      offset += bytesToCopy;

      pos0 = (pos0 + bytesToCopy) * (pos0 < preSieveTables[i+0].size());
      pos1 = (pos1 + bytesToCopy) * (pos1 < preSieveTables[i+1].size());
      pos2 = (pos2 + bytesToCopy) * (pos2 < preSieveTables[i+2].size());
      pos3 = (pos3 + bytesToCopy) * (pos3 < preSieveTables[i+3].size());
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
