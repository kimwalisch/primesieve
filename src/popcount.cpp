///
/// @file   popcount.cpp
/// @brief  Quickly count the number of 1 bits in an array.
///
/// Copyright (C) 2026 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/forward.hpp>
#include <primesieve/popcnt.hpp>
#include <primesieve/Vector.hpp>

#include <stdint.h>

/// For CPU architectures that have a POPCNT instruction, we use
/// that to count the number of 1 bits in the sieve array as
/// this will generally provide the best performance. For CPU
/// architectures without POPCNT we use the portable Harley-Seal
/// popcount algorithm further down.
///
#if defined(__POPCNT__) /* x86 GCC/Clang */ || \
    defined(ENABLE_MULTIARCH_x86_POPCNT) || \
   (defined(__ARM_NEON) || defined(__aarch64__))

namespace primesieve {

uint64_t popcount(const Vector<uint8_t>& array)
{
  ASSERT(array.capacity() % sizeof(uint64_t) == 0);
  uint64_t bytes = array.size();
  const uint8_t* data = array.data();

  constexpr uint64_t iter_bytes = sizeof(uint64_t) * 4;
  uint64_t limit = bytes - bytes % iter_bytes;
  uint64_t cnt = 0;
  uint64_t i;

  for(i = 0; i < limit; i += iter_bytes)
  {
    cnt += popcnt64(&data[i + 0 * sizeof(uint64_t)]);
    cnt += popcnt64(&data[i + 1 * sizeof(uint64_t)]);
    cnt += popcnt64(&data[i + 2 * sizeof(uint64_t)]);
    cnt += popcnt64(&data[i + 3 * sizeof(uint64_t)]);
  }
  for(; i < bytes; i += sizeof(uint64_t))
    cnt += popcnt64(&data[i]);

  return cnt;
}

} // namespace

#else

/// The "Harley-Seal popcount" algorithm that we use is a pure
/// integer algorithm that does not use the POPCNT instruction
/// present on many CPU architectures.
///
/// 1) This algorithm is portable (unlike POPCNT on x64)
///    and very fast, its speed is very close to POPCNT.
/// 2) Recent compilers can autovectorize this loop (e.g
///    using AVX512 on x64 CPUs) in which case this algorithm
///    will even outperform the POPCNT instruction.

namespace {

/// Carry-save adder (CSA).
/// @see Chapter 5 in "Hacker's Delight".
///
void CSA(uint64_t& h, uint64_t& l, uint64_t a, uint64_t b, uint64_t c)
{
  uint64_t u = a ^ b;
  h = (a & b) | (u & c);
  l = u ^ c;
}

} // namespace

namespace primesieve {

/// Harley-Seal popcount (4th iteration).
/// The Harley-Seal popcount algorithm is one of the fastest algorithms
/// for counting 1 bits in an array using only integer operations.
/// This implementation uses only 5.69 instructions per 64-bit word.
/// @see Chapter 5 in "Hacker's Delight" 2nd edition.
///
uint64_t popcount(const Vector<uint8_t>& array)
{
  ASSERT(array.capacity() % sizeof(uint64_t) == 0);
  uint64_t bytes = array.size();
  const uint8_t* data = array.data();

  uint64_t total = 0;
  uint64_t ones = 0, twos = 0, fours = 0, eights = 0, sixteens = 0;
  uint64_t twosA, twosB, foursA, foursB, eightsA, eightsB;
  constexpr uint64_t iter_bytes = sizeof(uint64_t) * 16;
  uint64_t limit = bytes - bytes % iter_bytes;
  uint64_t i = 0;

  for(; i < limit; i += iter_bytes)
  {
    CSA(twosA, ones, ones,
        load_aligned<uint64_t>(&data[i + 0 * sizeof(uint64_t)]),
        load_aligned<uint64_t>(&data[i + 1 * sizeof(uint64_t)]));
    CSA(twosB, ones, ones,
        load_aligned<uint64_t>(&data[i + 2 * sizeof(uint64_t)]),
        load_aligned<uint64_t>(&data[i + 3 * sizeof(uint64_t)]));
    CSA(foursA, twos, twos, twosA, twosB);
    CSA(twosA, ones, ones,
        load_aligned<uint64_t>(&data[i + 4 * sizeof(uint64_t)]),
        load_aligned<uint64_t>(&data[i + 5 * sizeof(uint64_t)]));
    CSA(twosB, ones, ones,
        load_aligned<uint64_t>(&data[i + 6 * sizeof(uint64_t)]),
        load_aligned<uint64_t>(&data[i + 7 * sizeof(uint64_t)]));
    CSA(foursB, twos, twos, twosA, twosB);
    CSA(eightsA,fours, fours, foursA, foursB);
    CSA(twosA, ones, ones,
        load_aligned<uint64_t>(&data[i + 8 * sizeof(uint64_t)]),
        load_aligned<uint64_t>(&data[i + 9 * sizeof(uint64_t)]));
    CSA(twosB, ones, ones,
        load_aligned<uint64_t>(&data[i + 10 * sizeof(uint64_t)]),
        load_aligned<uint64_t>(&data[i + 11 * sizeof(uint64_t)]));
    CSA(foursA, twos, twos, twosA, twosB);
    CSA(twosA, ones, ones,
        load_aligned<uint64_t>(&data[i + 12 * sizeof(uint64_t)]),
        load_aligned<uint64_t>(&data[i + 13 * sizeof(uint64_t)]));
    CSA(twosB, ones, ones,
        load_aligned<uint64_t>(&data[i + 14 * sizeof(uint64_t)]),
        load_aligned<uint64_t>(&data[i + 15 * sizeof(uint64_t)]));
    CSA(foursB, twos, twos, twosA, twosB);
    CSA(eightsB, fours, fours, foursA, foursB);
    CSA(sixteens, eights, eights, eightsA, eightsB);

    total += popcnt64(sixteens);
  }

  total *= 16;
  total += 8 * popcnt64(eights);
  total += 4 * popcnt64(fours);
  total += 2 * popcnt64(twos);
  total += 1 * popcnt64(ones);

  for(; i < bytes; i += sizeof(uint64_t))
    total += popcnt64(&data[i]);

  return total;
}

} // namespace

#endif
