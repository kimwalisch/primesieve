///
/// @file   pmath.hpp
/// @brief  Auxiliary math functions needed in primesieve.
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PMATH_HPP
#define PMATH_HPP

#include <stdint.h>
#include <algorithm>
#include <cmath>
#include <limits>

namespace {

template <typename X, typename Y>
inline X ceilDiv(X x, Y y)
{
  return (X) ((x + y - 1) / y);
}

template <typename T>
inline T numberOfBits(T)
{
  return (T) (sizeof(T) * 8);
}

template <typename T>
inline T isquare(T x)
{
  return x * x;
}

template <typename T>
inline bool isPowerOf2(T x)
{
  return x != 0 && (x & (x - 1)) == 0;
}

template <typename T>
inline T floorPowerOf2(T x)
{
  for (T i = 1; i < numberOfBits(x); i += i)
    x |= (x >> i);
  return x - (x >> 1);
}

template <typename T>
inline T ilog2(T x)
{
  T bits = numberOfBits(x);
  T one = 1;
  T log2 = 0;

  for (T i = bits / 2; i != 0; i /= 2)
  {
    if (x >= (one << i))
    {
      x >>= i;
      log2 += i;
    }
  }

  return log2;
}

/// Integer square root, Newton's method
/// @see book "Hacker's Delight"
///
template <typename T>
inline T isqrt(T x)
{
  if (x <= 1)
    return x;

  T bits = numberOfBits(x);
  T nlz = (bits - 1) - ilog2(x - 1);
  T s = bits / 2 - nlz / 2;
  T one = 1;

  // first guess: least power of 2 >= sqrt(x)
  T g0 = one << s;
  T g1 = (g0 + (x >> s)) >> 1;

  while (g1 < g0)
  {
    g0 = g1;
    g1 = (g0 + (x / g0)) >> 1;
  }

  return g0;
}

/// Returns 2^64-1 if x + y >= 2^64-1
inline uint64_t checkedAdd(uint64_t x, uint64_t y)
{
  const uint64_t max = std::numeric_limits<uint64_t>::max();

  if (x >= max - y)
    return max;

  return x + y;
}

/// Returns 0 if x - y <= 0
inline uint64_t checkedSub(uint64_t x, uint64_t y)
{
  return (x > y) ? x - y : 0;
}

template <typename A, typename B, typename C>
inline B inBetween(A min, B x, C max)
{
  if (x < (B) min)
    return (B) min;
  if ((C) x > max)
    return (B) max;

  return x;
}

/// Get an approximation of the maximum prime gap near n
template <typename T>
inline T max_prime_gap(T n)
{
  double x = (double) n;
  x = std::max(1.0, x);
  double logx = std::log(x);
  double prime_gap = logx * logx;
  return (T) prime_gap;
}

} // namespace

#endif
