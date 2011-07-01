/*
 * pmath.h -- This file is part of primesieve
 *
 * Copyright (C) 2011 Kim Walisch, <kim.walisch@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

/** 
 * @file pmath.h 
 * @brief Auxiliary math and bit manipulation functions needed in
 *        primesieve.
 */

#ifndef PMATH_H
#define PMATH_H

#include "defs.h"

#include <cstdlib>
#include <cassert>
#include <cmath>
#include <stdexcept>

/**
 * Count the number of 1 bits (population count) in an array using
 * Andrew Dalke's version of the 24 word bitslice algorithm.
 * Bitslice(24) is fast on 32 and 64-bit CPUs and a good choice if
 * hardware acceleration (SSE4.2 POPCNT) is not available.
 * http://www.dalkescientific.com/writings/diary/archive/2008/07/05/bitslice_and_popcount.html
 */
inline uint32_t popCount(const uint32_t* data, uint32_t size) {
  assert(data != NULL);
  assert(size <= UINT32_MAX / (8 * sizeof(uint32_t)));
  uint32_t i = 0;
  uint32_t bitCount = 0;

  // Bitslice(24)
  for (; i < size - size % 24; i += 24, data += 24) {
    uint32_t acc = 0;
    for (uint32_t j = 0; j < 24; j += 3) {
      uint32_t count1 = data[j];
      uint32_t count2 = data[j+1];
      uint32_t half1  = data[j+2];
      uint32_t half2  = data[j+2];
      half1  &= 0x55555555;
      half2   =            (half2  >> 1) & 0x55555555;
      count1  =  count1 - ((count1 >> 1) & 0x55555555);
      count2  =  count2 - ((count2 >> 1) & 0x55555555);
      count1 +=  half1;
      count2 +=  half2;
      count1  = (count1 & 0x33333333) + ((count1 >> 2) & 0x33333333);
      count1 += (count2 & 0x33333333) + ((count2 >> 2) & 0x33333333);
      acc    += (count1 & 0x0F0F0F0F) + ((count1 >> 4) & 0x0F0F0F0F);
    }
    acc = (acc & 0x00FF00FF) + ((acc >> 8) & 0x00FF00FF);
    acc =  acc + (acc >> 16);
    bitCount += acc & 0x0000FFFF;
  }

  // Count the bits of the remaining bytes (MAX 92) using "Counting
  // bits set, in parallel" from the "Bit Twiddling Hacks":
  // http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
  for (; i < size; i++, data++) {
    uint32_t v = *data;
    v =  v               - ((v >> 1) & 0x55555555);
    v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
    bitCount += ((v      +  (v >> 4) & 0x0F0F0F0F) * 0x01010101) >> 24;
  }
  return bitCount;
}

/**
 * Count the number of 1 bits (population count) in a small array
 * using "Counting bits set, Brian Kernighan's way" from the
 * "Bit Twiddling Hacks":
 * http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetKernighan
 */
inline uint32_t popCount(const uint8_t* data, uint32_t size) {
  assert(data != NULL);
  assert(size <= UINT32_MAX / (8 * sizeof(uint8_t)));
  uint32_t bitCount = 0;

  for (uint32_t i = 0; i < size; i++) {
    for (uint32_t v = data[i]; v != 0; v &= v - 1)
      bitCount++;
  }
  return bitCount;
}

/**
 * Round up to the next highest power of 2.
 * Code from: "Hacker's Delight, p. 48"
 */
inline uint32_t nextHighestPowerOf2(uint32_t x) {
  x = x - 1;
  x = x | (x >> 1);
  x = x | (x >> 2);
  x = x | (x >> 4);
  x = x | (x >> 8);
  x = x | (x >> 16);
  return x + 1;
}

/**
 * Determin if an integer is a power of 2.
 * Code from the "Bit Twiddling Hacks":
 * http://graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2
 */
inline bool isPowerOf2(uint32_t x) {
  return (x != 0 && (x & (x - 1)) == 0);
}

/**
 * Fast and protable integer log2 function.
 * Code from Juan Pablo:
 * http://www.southwindsgames.com/blog/2009/01/19/fast-integer-log2-function-in-cc/
 */
inline uint32_t floorLog2(uint32_t x) {
  uint32_t log2 = 0;
  if (x >= (1 << 16)) { x >>= 16; log2 |= 16; }
  if (x >= (1 <<  8)) { x >>=  8; log2 |=  8; }
  if (x >= (1 <<  4)) { x >>=  4; log2 |=  4; }
  if (x >= (1 <<  2)) { x >>=  2; log2 |=  2; }
  if (x >= (1 <<  1)) {           log2 |=  1; }
  return log2;
}

/**
 * Prime product function x#.
 * @pre x < 29
 * @return the product of the primes up to x
 */
inline uint32_t primeProduct(uint32_t x) {
  if (x >= 29)
    throw std::overflow_error("primeProduct(x): 32 bit overflow.");
  const uint32_t primes[9] = { 2, 3, 5, 7, 11, 13, 17, 19, 23 };
  uint32_t pp = 1;
  for (uint32_t i = 0; i < 9 && primes[i] <= x; i++)
    pp *= primes[i];
  return pp;
}

/**
 * Integer pow, raise to power, x^n.
 * Code from (ported to C++ from Ruby):
 * http://en.wikipedia.org/wiki/Exponentiation_by_squaring
 */
inline uint64_t ipow(uint64_t x, uint32_t n) {
  uint64_t result = 1;
  while (n != 0) {
    if (n & 1) {
      result *= x;
      n -= 1;
    }
    x *= x;
    n /= 2;
  }
  return result;
}

/**
 * Integer square root for 64-bit integers, x^(1/2).
 */
inline uint32_t isqrt(uint64_t x) {
  return static_cast<uint32_t> (std::sqrt(static_cast<double> (x)));
}

/**
 * Integer square, x^2.
 */
inline uint64_t isquare(uint32_t x) {
  return static_cast<uint64_t> (x) * static_cast<uint64_t> (x);
}

#endif /* PMATH_H */
