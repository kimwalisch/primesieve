///
/// @file   pmath.hpp
/// @brief  Auxiliary math functions for primesieve.
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
#include <cstddef>
#include <limits>
#include <type_traits>

namespace primesieve {

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
inline bool isPow2(T x)
{
  return x != 0 && (x & (x - 1)) == 0;
}

template <typename T>
inline T floorPow2(T x)
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

  for (T i = bits / 2; i > 0; i /= 2)
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
  using T = typename std::common_type<A, B, C>::type;

  if ((T) x < (T) min)
    return (B) min;
  if ((T) x > (T) max)
    return (B) max;

  return x;
}

/// prime_count_approx(x) >= pi(x)
inline std::size_t prime_count_approx(uint64_t start, uint64_t stop)
{
  if (start > stop)
    return 0;
  if (stop <= 10)
    return 4;

  // pi(x) <= x / (log(x) - 1.1) + 5, for x >= 4
  double x = (double) stop;
  double logx = std::log(x);
  double div = logx - 1.1;
  double pix = (stop - start) / div + 5;

  return (std::size_t) pix;
}

inline std::size_t prime_count_approx(uint64_t stop)
{
  return prime_count_approx(0, stop);
}

/// Approximation of the maximum prime gap near n
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
