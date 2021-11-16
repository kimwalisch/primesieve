///
/// @file   pmath.hpp
/// @brief  Auxiliary math functions for primesieve.
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
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

namespace {

template <typename X, typename Y>
inline X ceilDiv(X x, Y y)
{
  return (X) ((x + y - 1) / y);
}

template <typename T>
constexpr bool isPow2(T x)
{
  return x != 0 && (x & (x - 1)) == 0;
}

template <typename T>
constexpr T numberOfBits()
{
  return (T) std::numeric_limits<
      typename std::make_unsigned<T>::type
          >::digits;
}

template <typename T>
inline T floorPow2(T x)
{
  for (T i = 1; i < numberOfBits<T>(); i += i)
    x |= (x >> i);

  return x - (x >> 1);
}

template <typename T>
inline T ilog2(T x)
{
  T log2 = 0;
  T bits = numberOfBits<T>();

  for (T i = bits / 2; i > 0; i /= 2)
  {
    T one = 1;
    if (x >= (one << i))
    {
      x >>= i;
      log2 += i;
    }
  }

  return log2;
}

#if __cplusplus >= 201402L

/// C++14 compile time square root using binary search
template <typename T>
constexpr T ctSqrt(T x, T lo, T hi)
{
  if (lo == hi)
    return lo;

  const T mid = (lo + hi + 1) / 2;

  if (x / mid < mid)
    return ctSqrt<T>(x, lo, mid - 1);
  else
    return ctSqrt(x, mid, hi);
}

#else

#define MID ((lo + hi + 1) / 2)

/// C++11 compile time square root using binary search
template <typename T>
constexpr T ctSqrt(T x, T lo, T hi)
{
  return lo == hi ? lo : ((x / MID < MID)
      ? ctSqrt<T>(x, lo, MID - 1) : ctSqrt<T>(x, MID, hi));
}

#endif

template <typename T>
constexpr T ctSqrt(T x)
{
  return ctSqrt<T>(x, 0, x / 2 + 1);
}

template <typename T>
inline T isqrt(T x)
{
  T r = (T) std::sqrt((double) x);

  constexpr T maxSqrt = ctSqrt(std::numeric_limits<T>::max());
  r = std::min(r, maxSqrt);

  while (r * r > x)
    r--;
  while (x - r * r > r * 2)
    r++;

  return r;
}

/// Returns 2^64-1 if (x + y) > 2^64-1
inline uint64_t checkedAdd(uint64_t x, uint64_t y)
{
  if (x >= std::numeric_limits<uint64_t>::max() - y)
    return std::numeric_limits<uint64_t>::max();
  else
    return x + y;
}

/// Returns 0 if (x - y) < 0
inline uint64_t checkedSub(uint64_t x, uint64_t y)
{
  if (x > y)
    return x - y;
  else
    return 0;
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

/// primeCountApprox(x) >= pi(x)
inline std::size_t primeCountApprox(uint64_t start, uint64_t stop)
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

inline std::size_t primeCountApprox(uint64_t stop)
{
  return primeCountApprox(0, stop);
}

/// Approximation of the maximum prime gap near n
template <typename T>
inline T maxPrimeGap(T n)
{
  double x = (double) n;
  x = std::max(8.0, x);
  double logx = std::log(x);
  double prime_gap = logx * logx;

  return (T) prime_gap;
}

} // namespace

#endif
