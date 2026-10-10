///
/// @file PrimeGenerator_x86_avx2.hpp
///
/// Copyright (C) 2026 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRIMEGENERATOR_X86_AVX2_HPP
#define PRIMEGENERATOR_X86_AVX2_HPP

#include "PrimeGenerator.hpp"

#include <primesieve/macros.hpp>
#include <primesieve/Vector.hpp>

#include <stdint.h>
#include <immintrin.h>
#include <algorithm>
#include <cstddef>

namespace {

/// Bit values of the 1 bits of each sieve byte,
/// padded to 8 entries with zeros.
///
struct ByteBitValues
{
  uint16_t values[256][8];
};

constexpr ByteBitValues generateByteBitValues()
{
  ByteBitValues table = {};
  const uint16_t bitValues[8] = { 7, 11, 13, 17, 19, 23, 29, 31 };

  for (int byte = 0; byte < 256; byte++)
  {
    int i = 0;
    for (int bit = 0; bit < 8; bit++)
      if (byte & (1 << bit))
        table.values[byte][i++] = bitValues[bit];
  }

  return table;
}

alignas(64) constexpr ByteBitValues byteBitValues = generateByteBitValues();

constexpr uint64_t maxBlockWords = 256;
constexpr std::size_t maxBlockPrimes = 1024;

// The largest prime offset must fit into uint16_t
static_assert(240 * (maxBlockWords - 1) + 241 <= 0xffff,
              "Prime offsets must fit into uint16_t!");

/// This algorithm converts 1 bits from the sieve array into
/// primes using AVX2. The first loop uses a lookup table to
/// store 16-bit prime offsets. Overlapping stores discard
/// unused table entries. The second loop widens the offsets and
/// adds low to obtain 64-bit primes.
///
/// Returns the number of sieve words processed, stopping before
/// a word whose primes do not fit into the output buffer.
///
#if defined(ENABLE_MULTIARCH_AVX2_BMI2)
  PRIMESIEVE_MULTIARCH_KERNEL("avx2,bmi2,popcnt")
#endif
uint64_t sieveWordsToPrimes(const uint64_t* sieve,
                            uint64_t words,
                            uint64_t low,
                            uint64_t* primes,
                            std::size_t maxPrimes,
                            std::size_t* primeCount)
{
  ASSERT(words <= maxBlockWords);
  ASSERT(maxPrimes <= maxBlockPrimes);

  // +8 for the overlapping 8 x uint16_t stores
  alignas(16) INDETERMINATE uint16_t buffer[maxBlockPrimes + 8];

  // offsetsK = 30 * K + 240 * word (byte K of the word)
  __m128i offsets0 = _mm_set1_epi16(0);
  __m128i offsets1 = _mm_set1_epi16(30);
  __m128i offsets2 = _mm_set1_epi16(60);
  __m128i offsets3 = _mm_set1_epi16(90);
  __m128i offsets4 = _mm_set1_epi16(120);
  __m128i offsets5 = _mm_set1_epi16(150);
  __m128i offsets6 = _mm_set1_epi16(180);
  __m128i offsets7 = _mm_set1_epi16(210);
  __m128i offsetsStep = _mm_set1_epi16(8 * 30);

  uint64_t word = 0;
  std::size_t count = 0;

  // Pass 1: Store the prime offsets as uint16_t
  for (; word < words; word++)
  {
    uint64_t bits64 = sieve[word];
    std::size_t wordPrimes = _mm_popcnt_u64(bits64);

    // Prevent buffer overrun
    if (wordPrimes > maxPrimes - count)
      break;

    uint16_t* out = &buffer[count];

    // Store the prime offsets (bit values + offsets) of each
    // byte at its position, the position is the number of 1
    // bits below the byte. Each store overwrites the unused
    // table entries of the previous store.
    std::size_t pos = 0;
    std::size_t byte = bits64 & 0xff;
    __m128i bitValues = _mm_load_si128((const __m128i*) byteBitValues.values[byte]);
    _mm_storeu_si128((__m128i*) &out[pos], _mm_add_epi16(offsets0, bitValues));
    offsets0 = _mm_add_epi16(offsets0, offsetsStep);

    pos = _mm_popcnt_u64(_bzhi_u64(bits64, 8));
    byte = (bits64 >> 8) & 0xff;
    bitValues = _mm_load_si128((const __m128i*) byteBitValues.values[byte]);
    _mm_storeu_si128((__m128i*) &out[pos], _mm_add_epi16(offsets1, bitValues));
    offsets1 = _mm_add_epi16(offsets1, offsetsStep);

    pos = _mm_popcnt_u64(_bzhi_u64(bits64, 16));
    byte = (bits64 >> 16) & 0xff;
    bitValues = _mm_load_si128((const __m128i*) byteBitValues.values[byte]);
    _mm_storeu_si128((__m128i*) &out[pos], _mm_add_epi16(offsets2, bitValues));
    offsets2 = _mm_add_epi16(offsets2, offsetsStep);

    pos = _mm_popcnt_u64(_bzhi_u64(bits64, 24));
    byte = (bits64 >> 24) & 0xff;
    bitValues = _mm_load_si128((const __m128i*) byteBitValues.values[byte]);
    _mm_storeu_si128((__m128i*) &out[pos], _mm_add_epi16(offsets3, bitValues));
    offsets3 = _mm_add_epi16(offsets3, offsetsStep);

    pos = _mm_popcnt_u64(_bzhi_u64(bits64, 32));
    byte = (bits64 >> 32) & 0xff;
    bitValues = _mm_load_si128((const __m128i*) byteBitValues.values[byte]);
    _mm_storeu_si128((__m128i*) &out[pos], _mm_add_epi16(offsets4, bitValues));
    offsets4 = _mm_add_epi16(offsets4, offsetsStep);

    pos = _mm_popcnt_u64(_bzhi_u64(bits64, 40));
    byte = (bits64 >> 40) & 0xff;
    bitValues = _mm_load_si128((const __m128i*) byteBitValues.values[byte]);
    _mm_storeu_si128((__m128i*) &out[pos], _mm_add_epi16(offsets5, bitValues));
    offsets5 = _mm_add_epi16(offsets5, offsetsStep);

    pos = _mm_popcnt_u64(_bzhi_u64(bits64, 48));
    byte = (bits64 >> 48) & 0xff;
    bitValues = _mm_load_si128((const __m128i*) byteBitValues.values[byte]);
    _mm_storeu_si128((__m128i*) &out[pos], _mm_add_epi16(offsets6, bitValues));
    offsets6 = _mm_add_epi16(offsets6, offsetsStep);

    pos = _mm_popcnt_u64(_bzhi_u64(bits64, 56));
    byte = bits64 >> 56;
    bitValues = _mm_load_si128((const __m128i*) byteBitValues.values[byte]);
    _mm_storeu_si128((__m128i*) &out[pos], _mm_add_epi16(offsets7, bitValues));
    offsets7 = _mm_add_epi16(offsets7, offsetsStep);

    count += wordPrimes;
  }

  // Pass 2: Widen the prime offsets into 64-bit primes
  __m256i base = _mm256_set1_epi64x(low);
  std::size_t i = 0;

  for (; i + 4 <= count; i += 4)
  {
    __m256i offsets64 = _mm256_cvtepu16_epi64(_mm_loadl_epi64((const __m128i*) &buffer[i]));
    _mm256_storeu_si256((__m256i*) &primes[i], _mm256_add_epi64(base, offsets64));
  }

  NO_UNROLL_LOOP
  NO_VECTORIZE_LOOP
  for (; i < count; i++)
    primes[i] = low + buffer[i];

  *primeCount = count;
  return word;
}

} // namespace

namespace primesieve {

/// This method is used by iterator::next_prime().
/// Stores the next few primes (~ 1000) in the primes vector.
///
#if defined(ENABLE_MULTIARCH_AVX2_BMI2)
  PRIMESIEVE_MULTIARCH_KERNEL("avx2,bmi2,popcnt")
#endif
void PrimeGenerator::fillNextPrimes_x86_avx2(Vector<uint64_t>& primes, std::size_t* size)
{
  *size = 0;

  do
  {
    if (sieveIdx_ >= sieve_.size())
      if (!sieveNextPrimes(primes, size))
        return;

    // Use local variables to prevent the compiler from
    // writing temporary results to memory.
    std::size_t i = *size;
    std::size_t maxSize = primes.size();
    ASSERT(i + 64 <= maxSize);
    uint64_t low = low_;
    uint64_t sieveIdx = sieveIdx_;
    uint64_t sieveSize = sieve_.size();
    const uint64_t* sieve = sieve_.data();

    while (sieveIdx < sieveSize)
    {
      std::size_t primeCount;
      uint64_t words = std::min(sieveSize - sieveIdx, maxBlockWords);
      std::size_t maxPrimes = std::min(maxSize - i, maxBlockPrimes);
      uint64_t processed = sieveWordsToPrimes(&sieve[sieveIdx], words, low,
          primes.data() + i, maxPrimes, &primeCount);

      i += primeCount;
      low += processed * 240;
      sieveIdx += processed;

      // The next sieve word does not fit
      // into the primes buffer anymore.
      if (processed < words)
        break;
    }

    low_ = low;
    sieveIdx_ = sieveIdx;
    *size = i;
  }
  while (*size == 0);
}

/// This method is used by iterator::prev_prime().
/// Stores primes in ascending order for backward iteration.
///
#if defined(ENABLE_MULTIARCH_AVX2_BMI2)
  PRIMESIEVE_MULTIARCH_KERNEL("avx2,bmi2,popcnt")
#endif
void PrimeGenerator::fillPrevPrimes_x86_avx2(Vector<uint64_t>& primes, std::size_t* size)
{
  *size = 0;

  while (sievePrevPrimes(primes, size))
  {
    // Use local variables to prevent the compiler from
    // writing temporary results to memory.
    std::size_t i = *size;
    uint64_t low = low_;
    uint64_t sieveIdx = sieveIdx_;
    uint64_t sieveSize = sieve_.size();
    const uint64_t* sieve = sieve_.data();

    while (sieveIdx < sieveSize)
    {
      // Each iteration generates up to maxBlockPrimes primes
      if_unlikely(i + maxBlockPrimes > primes.size())
        primes.resize(i + maxBlockPrimes);

      std::size_t primeCount;
      uint64_t words = std::min(sieveSize - sieveIdx, maxBlockWords);
      words = sieveWordsToPrimes(&sieve[sieveIdx], words, low,
          &primes[i], maxBlockPrimes, &primeCount);

      i += primeCount;
      low += words * 240;
      sieveIdx += words;
    }

    low_ = low;
    sieveIdx_ = sieveIdx;
    *size = i;
  }
}

} // namespace

#endif
