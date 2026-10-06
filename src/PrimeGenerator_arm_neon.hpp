///
/// @file PrimeGenerator_arm_neon.hpp
///
/// Copyright (C) 2026 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRIMEGENERATOR_ARM_NEON_HPP
#define PRIMEGENERATOR_ARM_NEON_HPP

#include "PrimeGenerator.hpp"

#include <primesieve/macros.hpp>
#include <primesieve/util.hpp>
#include <primesieve/Vector.hpp>

#include <stdint.h>
#include <arm_neon.h>
#include <algorithm>
#include <cstddef>

namespace {

/// Bit values of each sieve byte,
/// padded to 8 entries with zeros.
///
struct ByteBitValues
{
  uint8_t values[256 * 8];
};

constexpr ByteBitValues generateByteBitValues()
{
  ByteBitValues table = {};
  const uint8_t bitValues[8] = { 7, 11, 13, 17, 19, 23, 29, 31 };

  for (int byte = 0; byte < 256; byte++)
  {
    int i = 0;
    for (int bit = 0; bit < 8; bit++)
      if (byte & (1 << bit))
        table.values[byte * 8 + i++] = bitValues[bit];
  }

  return table;
}

alignas(64) constexpr ByteBitValues byteBitValues = generateByteBitValues();

// Keep offsets within uint16_t: 240 * 255 + 241 = 61441
constexpr uint64_t maxBlockWords = 256;
constexpr std::size_t maxBlockPrimes = 1024;

/// This algorithm converts 1 bits from the sieve array into
/// primes using ARM NEON. The first loop uses a lookup table to
/// store 16-bit prime offsets. Overlapping stores discard
/// unused table entries. The second loop widens the offsets and
/// adds low to obtain 64-bit primes.
///
/// Returns the number of sieve words processed, stopping before
/// a word whose primes do not fit into the output buffer.
///
ALWAYS_INLINE uint64_t sieveWordsToPrimes(const uint64_t* sieve,
                                          uint64_t words,
                                          uint64_t low,
                                          uint64_t* primes,
                                          std::size_t maxPrimes,
                                          std::size_t* primeCount)
{
  ASSERT(words <= maxBlockWords);
  ASSERT(maxPrimes <= maxBlockPrimes);

  // +8 for the overlapping 8 x uint16_t stores
  INDETERMINATE alignas(16) uint16_t buffer[maxBlockPrimes + 8];
  std::size_t count = 0;
  uint64_t word = 0;

  uint16x8_t offsets0 = vdupq_n_u16(30 * 0);
  uint16x8_t offsets1 = vdupq_n_u16(30 * 1);
  uint16x8_t offsets2 = vdupq_n_u16(30 * 2);
  uint16x8_t offsets3 = vdupq_n_u16(30 * 3);
  uint16x8_t offsets4 = vdupq_n_u16(30 * 4);
  uint16x8_t offsets5 = vdupq_n_u16(30 * 5);
  uint16x8_t offsets6 = vdupq_n_u16(30 * 6);
  uint16x8_t offsets7 = vdupq_n_u16(30 * 7);
  uint16x8_t offsetsStep = vdupq_n_u16(8 * 30);

  // Pass 1: Store the prime offsets as uint16_t
  for (; word < words; word++)
  {
    uint64_t bits64 = primesieve::to_littleendian(sieve[word]);

    // Byte i of prefix = number of 1 bits in bytes 0..i
    // = position (in the buffer) of byte i + 1's bit values.
    uint8x8_t byteCounts = vcnt_u8(vcreate_u8(bits64));
    uint64_t prefix = vget_lane_u64(vreinterpret_u64_u8(byteCounts), 0) * 0x0101010101010101ull;
    uint64_t wordPrimes = prefix >> 56;

    // Prevent buffer overrun
    if (wordPrimes > maxPrimes - count)
      break;

    uint16_t* out = &buffer[count];
    count += wordPrimes;

    // Bit values of the 1 bits of each byte
    uint8x8_t bitValues0 = vld1_u8(&byteBitValues.values[((bits64 >>  0) & 0xff) * 8]);
    uint8x8_t bitValues1 = vld1_u8(&byteBitValues.values[((bits64 >>  8) & 0xff) * 8]);
    uint8x8_t bitValues2 = vld1_u8(&byteBitValues.values[((bits64 >> 16) & 0xff) * 8]);
    uint8x8_t bitValues3 = vld1_u8(&byteBitValues.values[((bits64 >> 24) & 0xff) * 8]);
    uint8x8_t bitValues4 = vld1_u8(&byteBitValues.values[((bits64 >> 32) & 0xff) * 8]);
    uint8x8_t bitValues5 = vld1_u8(&byteBitValues.values[((bits64 >> 40) & 0xff) * 8]);
    uint8x8_t bitValues6 = vld1_u8(&byteBitValues.values[((bits64 >> 48) & 0xff) * 8]);
    uint8x8_t bitValues7 = vld1_u8(&byteBitValues.values[((bits64 >> 56) & 0xff) * 8]);

    // Store the prime offsets (bit values + offsets) of each
    // byte at its position. Each store overwrites the unused
    // table entries of the previous store.
    vst1q_u16(out, vaddw_u8(offsets0, bitValues0));
    vst1q_u16(out + ((prefix >>  0) & 0xff), vaddw_u8(offsets1, bitValues1));
    vst1q_u16(out + ((prefix >>  8) & 0xff), vaddw_u8(offsets2, bitValues2));
    vst1q_u16(out + ((prefix >> 16) & 0xff), vaddw_u8(offsets3, bitValues3));
    vst1q_u16(out + ((prefix >> 24) & 0xff), vaddw_u8(offsets4, bitValues4));
    vst1q_u16(out + ((prefix >> 32) & 0xff), vaddw_u8(offsets5, bitValues5));
    vst1q_u16(out + ((prefix >> 40) & 0xff), vaddw_u8(offsets6, bitValues6));
    vst1q_u16(out + ((prefix >> 48) & 0xff), vaddw_u8(offsets7, bitValues7));

    offsets0 = vaddq_u16(offsets0, offsetsStep);
    offsets1 = vaddq_u16(offsets1, offsetsStep);
    offsets2 = vaddq_u16(offsets2, offsetsStep);
    offsets3 = vaddq_u16(offsets3, offsetsStep);
    offsets4 = vaddq_u16(offsets4, offsetsStep);
    offsets5 = vaddq_u16(offsets5, offsetsStep);
    offsets6 = vaddq_u16(offsets6, offsetsStep);
    offsets7 = vaddq_u16(offsets7, offsetsStep);
  }

  // Pass 2: Widen the prime offsets into 64-bit primes
  uint64x2_t base = vdupq_n_u64(low);
  std::size_t i = 0;

  if (count >= 8)
  {
    const uint16_t* buffer16 = buffer;
    const uint16_t* end = &buffer[count - 8];
    uint64_t* primes64 = primes;

    do
    {
      uint16x8_t offsets = vld1q_u16(buffer16);
      uint32x4_t offsets0 = vmovl_u16(vget_low_u16(offsets));
      uint32x4_t offsets1 = vmovl_high_u16(offsets);
      vst1q_u64(&primes64[0], vaddw_u32(base, vget_low_u32(offsets0)));
      vst1q_u64(&primes64[2], vaddw_high_u32(base, offsets0));
      vst1q_u64(&primes64[4], vaddw_u32(base, vget_low_u32(offsets1)));
      vst1q_u64(&primes64[6], vaddw_high_u32(base, offsets1));
      buffer16 += 8;
      primes64 += 8;
    }
    while (buffer16 <= end);

    i = buffer16 - buffer;
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
void PrimeGenerator::fillNextPrimes_arm_neon(Vector<uint64_t>& primes, std::size_t* size)
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
      words = sieveWordsToPrimes(&sieve[sieveIdx], words, low, primes.data() + i, maxPrimes, &primeCount);

      // The primes array is full
      if (words == 0)
        break;

      i += primeCount;
      low += words * 240;
      sieveIdx += words;
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
void PrimeGenerator::fillPrevPrimes_arm_neon(Vector<uint64_t>& primes, std::size_t* size)
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
      words = sieveWordsToPrimes(&sieve[sieveIdx], words, low, &primes[i], maxBlockPrimes, &primeCount);
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
