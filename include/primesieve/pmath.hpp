///
/// @file   pmath.hpp
/// @brief  Auxiliary math functions needed in primesieve.
///
/// Copyright (C) 2016 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PMATH_HPP
#define PMATH_HPP

#include <cmath>
#include <limits>

namespace primesieve {

template <typename X, typename Y>
inline X ceilDiv(X x, Y y)
{
  return static_cast<X>((x + y - 1) / y);
}

template <typename T>
inline T numberOfBits(T)
{
  return static_cast<T>(sizeof(T) * 8);
}

template <typename T>
inline T isquare(T x)
{
  return x * x;
}

/// Check if an integer is a power of 2
template <typename T>
inline bool isPowerOf2(T x)
{
  return (x != 0 && (x & (x - 1)) == 0);
}

/// Round down to the next power of 2
template <typename T>
inline T floorPowerOf2(T x)
{
  for (T i = 1; i < numberOfBits(x); i += i)
    x |= (x >> i);
  return x - (x >> 1);
}

/// Fast and protable integer log2 function
template <typename T>
inline T ilog2(T x)
{
  const T bits = numberOfBits(x);
  const T one = 1;
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
template <typename T>
inline T isqrt(T x)
{
  if (x <= 1)
    return x;

  const T bits = numberOfBits(x);
  const T one = 1;

  // s = bits / 2 - nlz(x - 1) / 2
  // nlz(x) = bits - 1 - ilog2(x)
  T s = bits / 2 - (bits - 1) / 2 + ilog2(x - 1) / 2;

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

template <typename T1, typename T2, typename T3>
inline T2 inBetween(T1 min, T2 x, T3 max)
{
  if (x < (T2) min) return (T2) min;
  if (x > (T2) max) return (T2) max;

  return x;
}

/// Returns a+b or 2^64-1 if the result overflows
inline uint64_t add_overflow_safe(uint64_t x, uint64_t y)
{
  if (x >= std::numeric_limits<uint64_t>::max() - y)
    return std::numeric_limits<uint64_t>::max();

  return x + y;
}

/// Returns a-b or 0 if the result underflows
inline uint64_t sub_underflow_safe(uint64_t x, uint64_t y)
{
  return (x > y) ? x - y : 0;
}

/// Get an approximation of the maximum prime gap near n
inline uint64_t max_prime_gap(uint64_t n)
{
  double x = static_cast<double>(n);
  double logx = std::log(x);
  double prime_gap = logx * logx;
  return static_cast<uint64_t>(prime_gap);
}

} // namespace

#endif
