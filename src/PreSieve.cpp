///
/// @file   PreSieve.cpp
/// @brief  Pre-sieve multiples of small primes < 100 to speed up the
///         sieve of Eratosthenes. The idea is to allocate several
///         arrays (buffers) and remove the multiples of small primes
///         from them at initialization. Each buffer is assigned
///         different primes, for example:
///
///         buffer[0] removes multiplies of: {  7, 67, 71 } // 32 KiB
///         buffer[1] removes multiplies of: { 11, 41, 73 } // 32 KiB
///         buffer[2] removes multiplies of: { 13, 43, 59 } // 32 KiB
///         buffer[3] removes multiplies of: { 17, 37, 53 } // 32 KiB
///         buffer[4] removes multiplies of: { 19, 29, 61 } // 32 KiB
///         buffer[5] removes multiplies of: { 23, 31, 47 } // 32 KiB
///         buffer[6] removes multiplies of: { 79, 97 }     // 30 KiB
///         buffer[7] removes multiplies of: { 83, 89 }     // 29 KiB
///
///         Then whilst sieving, we perform a bitwise AND on the
///         buffers arrays and store the result in the sieve array.
///         Pre-sieving provides a speedup of up to 30% when
///         sieving the primes < 10^10 using primesieve.
///
/// Copyright (C) 2024 Kim Walisch, <kim.walisch@gmail.com>
/// Copyright (C) 2022 @zielaj, https://github.com/zielaj
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/PreSieve.hpp>
#include <primesieve/PreSieve_Tables.hpp>
#include <primesieve/Vector.hpp>
#include <primesieve/macros.hpp>

#include <stdint.h>
#include <algorithm>

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

namespace {

#if defined(HAS_SSE2)

/// Since compiler auto-vectorization is not 100% reliable, we have
/// manually vectorized the andBuffers() function for x64 CPUs.
/// This algorithm is portable since all x64 CPUs support the SSE2
/// instruction set.
///
void andBuffers(const uint8_t* __restrict buf0,
                const uint8_t* __restrict buf1,
                const uint8_t* __restrict buf2,
                const uint8_t* __restrict buf3,
                const uint8_t* __restrict buf4,
                const uint8_t* __restrict buf5,
                const uint8_t* __restrict buf6,
                const uint8_t* __restrict buf7,
                uint8_t* __restrict output,
                std::size_t bytes)
{
  std::size_t i = 0;
  std::size_t limit = bytes - bytes % sizeof(__m128i);

  // Note that I also tried vectorizing this algorithm using AVX2
  // which has double the vector width compared to SSE2, but this did
  // not provide any speedup. On average, this loop processes only
  // 2192 bytes, hence there aren't many vector loop iterations and
  // by increasing the vector width this also increases the number of
  // scalar loop iterations after the vector loop finishes which
  // could potentially even become a bottleneck.
  for (; i < limit; i += sizeof(__m128i))
  {
    _mm_storeu_si128((__m128i*) &output[i],
        _mm_and_si128(
            _mm_and_si128(
                _mm_and_si128(_mm_loadu_si128((const __m128i*) &buf0[i]), _mm_loadu_si128((const __m128i*) &buf1[i])),
                _mm_and_si128(_mm_loadu_si128((const __m128i*) &buf2[i]), _mm_loadu_si128((const __m128i*) &buf3[i]))),
            _mm_and_si128(
                _mm_and_si128(_mm_loadu_si128((const __m128i*) &buf4[i]), _mm_loadu_si128((const __m128i*) &buf5[i])),
                _mm_and_si128(_mm_loadu_si128((const __m128i*) &buf6[i]), _mm_loadu_si128((const __m128i*) &buf7[i])))));
  }

  for (; i < bytes; i++)
    output[i] = buf0[i] & buf1[i] & buf2[i] & buf3[i] &
                buf4[i] & buf5[i] & buf6[i] & buf7[i];
}

#elif defined(HAS_ARM_NEON)

/// Homebrew compiles its C/C++ packages on macOS using Clang -Os
/// (instead of -O2 or -O3) which does not auto-vectorize our simple
/// loop with Bitwise AND. If this loop is not vectorized this
/// deteriorates the performance of primesieve by up to 40%. As a
/// workaround for this Homebrew issue we have manually vectorized
/// the Bitwise AND loop using ARM NEON.
///
void andBuffers(const uint8_t* __restrict buf0,
                const uint8_t* __restrict buf1,
                const uint8_t* __restrict buf2,
                const uint8_t* __restrict buf3,
                const uint8_t* __restrict buf4,
                const uint8_t* __restrict buf5,
                const uint8_t* __restrict buf6,
                const uint8_t* __restrict buf7,
                uint8_t* __restrict output,
                std::size_t bytes)
{
  std::size_t i = 0;
  std::size_t limit = bytes - bytes % sizeof(uint8x16_t);

  for (; i < limit; i += sizeof(uint8x16_t))
  {
    vst1q_u8(&output[i],
        vandq_u8(
            vandq_u8(
                vandq_u8(vld1q_u8(&buf0[i]), vld1q_u8(&buf1[i])),
                vandq_u8(vld1q_u8(&buf2[i]), vld1q_u8(&buf3[i]))),
            vandq_u8(
                vandq_u8(vld1q_u8(&buf4[i]), vld1q_u8(&buf5[i])),
                vandq_u8(vld1q_u8(&buf6[i]), vld1q_u8(&buf7[i])))));
  }

  for (; i < bytes; i++)
    output[i] = buf0[i] & buf1[i] & buf2[i] & buf3[i] &
                buf4[i] & buf5[i] & buf6[i] & buf7[i];
}

#else

void andBuffers(const uint8_t* __restrict buf0,
                const uint8_t* __restrict buf1,
                const uint8_t* __restrict buf2,
                const uint8_t* __restrict buf3,
                const uint8_t* __restrict buf4,
                const uint8_t* __restrict buf5,
                const uint8_t* __restrict buf6,
                const uint8_t* __restrict buf7,
                uint8_t* __restrict output,
                std::size_t bytes)
{
  // This loop will get auto-vectorized if compiled with GCC/Clang
  // using -O3. Using GCC -O2 does not auto-vectorize this loop
  // because -O2 uses the "very-cheap" vector cost model. To fix
  // this issue we enable -ftree-vectorize -fvect-cost-model=dynamic
  // if the compiler supports it in auto_vectorization.cmake.
  for (std::size_t i = 0; i < bytes; i++)
    output[i] = buf0[i] & buf1[i] & buf2[i] & buf3[i] &
                buf4[i] & buf5[i] & buf6[i] & buf7[i];
}

#endif

} // namespace

namespace primesieve {

void PreSieve::preSieve(Vector<uint8_t>& sieve, uint64_t segmentLow)
{
  uint64_t offset = 0;
  Array<uint64_t, 8> pos;

  for (std::size_t i = 0; i < buffers.size(); i++)
    pos[i] = (segmentLow % (buffers[i].size() * 30)) / 30;

  while (offset < sieve.size()) {
    uint64_t bytesToCopy = sieve.size() - offset;

    for (std::size_t i = 0; i < buffers.size(); i++) {
      uint64_t left = buffers[i].size() - pos[i];
      bytesToCopy = std::min(left, bytesToCopy);
    }

    andBuffers(&*(buffers[0].begin() + pos[0]), // &buffer[0][pos[0]]
               &*(buffers[1].begin() + pos[1]), // &buffer[1][pos[1]]
               &*(buffers[2].begin() + pos[2]), // &buffer[2][pos[2]]
               &*(buffers[3].begin() + pos[3]), // &buffer[3][pos[3]]
               &*(buffers[4].begin() + pos[4]), // &buffer[4][pos[4]]
               &*(buffers[5].begin() + pos[5]), // &buffer[5][pos[5]]
               &*(buffers[6].begin() + pos[6]), // &buffer[6][pos[6]]
               &*(buffers[7].begin() + pos[7]), // &buffer[7][pos[7]]
               &sieve[offset],
               bytesToCopy);

    offset += bytesToCopy;

    for (std::size_t i = 0; i < pos.size(); i++) {
      pos[i] += bytesToCopy;
      if (pos[i] >= buffers[i].size())
        pos[i] = 0;
    }
  }

  // Pre-sieving removes the primes < 100. We
  // have to undo that work and reset these bits
  // to 1 (but 49 = 7 * 7 is not a prime).
  if (segmentLow < 120)
  {
    uint64_t i = segmentLow / 30;
    uint8_t* sieveArray = sieve.data();
    Array<uint8_t, 8> primeBits = { 0xff, 0xef, 0x77, 0x3f, 0xdb, 0xed, 0x9e, 0xfc };

    ASSERT(sieve.capacity() >= 4);
    for (std::size_t j = 0; j < 4; j++)
      sieveArray[j] = primeBits[i + j];
  }
}

} // namespace
