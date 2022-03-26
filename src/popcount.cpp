///
/// @file   popcount.cpp
/// @brief  Quickly count the number of 1 bits in an array.
///
///         The "Harley-Seal popcount" algorithm that we use is a pure
///         integer algorithm that does not use the POPCNT instruction
///         present on many CPU architectures. There are a few reasons
///         why we do not use the POPCNT instruction here:
///
///         1) This algorithm is not really a bottleneck.
///         2) This algorithm is portable (unlike POPCNT on x64)
///            and very fast, its speed is very close to POPCNT.
///         3) Recent compilers can autovectorize this loop (e.g
///            using AVX512 on x64 CPUs) in which case this algorithm
///            will even outperform the POPCNT instruction.
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/intrinsics.hpp>
#include <primesieve/forward.hpp>
#include <cstdint>

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
uint64_t popcount(const uint64_t* array, uint64_t size)
{
  uint64_t total = 0;
  uint64_t ones = 0, twos = 0, fours = 0, eights = 0, sixteens = 0;
  uint64_t twosA, twosB, foursA, foursB, eightsA, eightsB;
  uint64_t limit = size - size % 16;
  uint64_t i = 0;

  for(; i < limit; i += 16)
  {
    CSA(twosA, ones, ones, array[i+0], array[i+1]);
    CSA(twosB, ones, ones, array[i+2], array[i+3]);
    CSA(foursA, twos, twos, twosA, twosB);
    CSA(twosA, ones, ones, array[i+4], array[i+5]);
    CSA(twosB, ones, ones, array[i+6], array[i+7]);
    CSA(foursB, twos, twos, twosA, twosB);
    CSA(eightsA,fours, fours, foursA, foursB);
    CSA(twosA, ones, ones, array[i+8], array[i+9]);
    CSA(twosB, ones, ones, array[i+10], array[i+11]);
    CSA(foursA, twos, twos, twosA, twosB);
    CSA(twosA, ones, ones, array[i+12], array[i+13]);
    CSA(twosB, ones, ones, array[i+14], array[i+15]);
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

  for(; i < size; i++)
    total += popcnt64(array[i]);

  return total;
}

} // namespace
