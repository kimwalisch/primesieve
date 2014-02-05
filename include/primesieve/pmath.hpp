///
/// @file   pmath.hpp
/// @brief  Auxiliary math functions needed in primesieve.
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PMATH_HPP
#define PMATH_HPP

#include <cmath>

namespace primesieve {

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

/// @brief  Check if an integer is a power of 2.
/// @see    Book "Hacker's Delight".
///
template <typename T>
inline bool isPowerOf2(T x)
{
  return (x != 0 && (x & (x - 1)) == 0);
}

/// @brief  Round down to the next power of 2.
/// @see    Book "Hacker's Delight".
///
template <typename T>
inline T floorPowerOf2(T x)
{
  for (T i = 1; i < numberOfBits(x); i += i)
    x |= (x >> i);
  return x - (x >> 1);
}

/// @brief  Fast and protable integer log2 function.
/// @see    Book "Hacker's Delight".
///
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

/// @brief  Integer square root, Newton's method.
/// @see    Book "Hacker's Delight".
///
template <typename T>
inline T isqrt(T x)
{
  if (x <= 1)
    return x;

  const T bits = numberOfBits(x);
  const T one = 1;

  // s      = bits / 2 - nlz(x - 1) / 2
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

template <typename T>
inline T getInBetween(T min, T value, T max)
{
  if (value < min) return min;
  if (value > max) return max;
  return value;
}

/// @brief   Get an approximation of the maximum prime gap near n.
/// @return  log(n)^2
///
inline uint64_t max_prime_gap(uint64_t n)
{
  double logn = std::log(static_cast<double>(n));
  double prime_gap = logn * logn;
  return static_cast<uint64_t>(prime_gap);
}

} // namespace primesieve

#endif
