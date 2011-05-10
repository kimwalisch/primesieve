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
 * @brief Auxiliary math functions needed in primesieve.
 */

#ifndef PMATH_H
#define PMATH_H

#include "defs.h"

#include <stdexcept>
#include <cmath>

/**
 * Number of trailing zeros, simple counting loops.
 * Code from: "Hacker's Delight"
 */
inline uint32_t ntz(uint32_t x) {
  x = ~x & (x - 1);
  uint32_t n = 0;
  while (x != 0) {
    n = n + 1;
    x = x >> 1;
  }
  return n;
}

/**
 * Rounds up to the next highest power of 2.
 * Code from: "Hacker's Delight"
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
 * Determins if an integer is a power of 2.
 * Code from the "Bit Twiddling Hacks":
 * http://graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2
 */
inline bool isPowerOf2(uint32_t x) {
  return (x && !(x & (x - 1)));
}

/**
 * Fast and protable integer log2 function.
 * Code from:
 * http://www.southwindsgames.com/blog/2009/01/19/fast-integer-log2-function-in-cc/
 */
inline uint32_t floorLog2(uint32_t x) {
  uint32_t log2 = 0;
  if (x >= (1 << 16)) {
    x >>= 16;
    log2 |= 16;
  }
  if (x >= (1 << 8)) {
    x >>= 8;
    log2 |= 8;
  }
  if (x >= (1 << 4)) {
    x >>= 4;
    log2 |= 4;
  }
  if (x >= (1 << 2)) {
    x >>= 2;
    log2 |= 2;
  }
  if (x >= (1 << 1))
    log2 |= 1;
  return log2;
}

/**
 * Prime product function (x)#.
 * @pre x < 29.
 * @return the product of the prime numbers up to x.
 */
inline uint32_t primeProduct(uint32_t x) {
  if (x >= 29)
    throw std::overflow_error("primeProduct: 32 bit overflow.");
  const uint32_t smallPrimes[9] = { 2, 3, 5, 7, 11, 13, 17, 19, 23 };
  uint32_t pp = 1;
  for (uint32_t i = 0; i < 9 && smallPrimes[i] <= x; i++)
    pp *= smallPrimes[i];
  return pp;
}

/**
 * Integer pow, raise to power.
 * @return base raised to the power exponent.
 * Code from (adapted):
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
 * Integer square root for 64-bit integers.
 * @return (uint32_t) floor(sqrt(x)).
 */
inline uint32_t isqrt(uint64_t x) {
  return static_cast<uint32_t> (std::sqrt(static_cast<double> (x)));
}

/**
 * Integer square root for 32-bit integers.
 * @return (uint32_t) floor(sqrt(x)).
 */
inline uint32_t isqrt(uint32_t x) {
  return static_cast<uint32_t> (std::sqrt(static_cast<double> (x)));
}

/**
 * Integer square, x^2.
 */
inline uint64_t isquare(uint32_t x) {
  return static_cast<uint64_t> (x) * static_cast<uint64_t> (x);
}

#endif /* PMATH_H */
