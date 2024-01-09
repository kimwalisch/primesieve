///
/// @file   pmath.hpp
/// @brief  Auxiliary math functions for primesieve.
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
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

#if __cplusplus >= 202002L
  #include <bit>
#endif

#if !defined(__has_builtin)
  #define __has_builtin(x) 0
#endif

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
#if __cplusplus >= 202002L
  if (x == 0)
    return 0;
  auto ux = std::make_unsigned_t<T>(x);
  return ((T) 1) << (std::bit_width(ux) - 1);

#elif __has_builtin(__builtin_clzll)
  if (x == 0)
    return 0;
  static_assert(sizeof(T) <= sizeof(unsigned long long), "Unsupported type, wider than long long!");
  auto bits = numberOfBits<unsigned long long>();
  T ilog2_x = (T) ((bits - 1) - __builtin_clzll(x));
  return ((T) 1) << ilog2_x;

#else
  for (T i = 1; i < numberOfBits<T>(); i += i)
    x |= (x >> i);
  return x - (x >> 1);
#endif
}

template <typename T>
inline T ilog2(T x)
{
#if __cplusplus >= 202002L
  auto ux = std::make_unsigned_t<T>(x);
  ux = (ux > 0) ? ux : 1;
  return std::bit_width(ux) - 1;

#elif __has_builtin(__builtin_clzll)
  static_assert(sizeof(T) <= sizeof(unsigned long long), "Unsupported type, wider than long long!");
  auto bits = numberOfBits<unsigned long long>();

  // Workaround to avoid undefined behavior,
  // __builtin_clz(0) is undefined.
  x = (x > 0) ? x : 1;
  return (T) ((bits - 1) - __builtin_clzll(x));

#else
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
#endif
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

/// primeCountUpper(x) >= pi(x).
/// In order to prevent having to resize vectors with prime numbers
/// (which would incur additional overhead) it is important that
/// primeCountUpper(x) >= pi(x). Also for our purpose, it is
/// actually beneficial if primeCountUpper(x) is a few percent
/// larger (e.g. 3%) than pi(x), this reduces the number of memory
/// allocations in PrimeGenerator::fillPrevPrimes().
///
inline std::size_t primeCountUpper(uint64_t start, uint64_t stop)
{
  if (start > stop)
    return 0;

  // pi(x) <= x / (log(x) - 1.1) + 5, for x >= 4.
  // Pierre Dusart, https://arxiv.org/abs/1002.0442 eq. 6.6.
  double x = std::max(100.0, (double) stop);
  double pix = (stop - start) / (std::log(x) - 1.1) + 5;

  // This can only happen on 32-bit OSes
  if (pix > (double) std::numeric_limits<std::size_t>::max())
    return std::numeric_limits<std::size_t>::max();

  return (std::size_t) pix;
}

inline std::size_t primeCountUpper(uint64_t stop)
{
  return primeCountUpper(0, stop);
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

#if __cplusplus >= 202002L

/// C++20 compile time square root using binary search
template <typename T>
consteval T ctSqrt(T x, T lo, T hi)
{
  if (lo == hi)
    return lo;

  const T mid = (lo + hi + 1) / 2;

  if (x / mid < mid)
    return ctSqrt<T>(x, lo, mid - 1);
  else
    return ctSqrt(x, mid, hi);
}

template <typename T>
consteval T ctSqrt(T x)
{
  return ctSqrt<T>(x, 0, x / 2 + 1);
}

#elif __cplusplus >= 201402L

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

template <typename T>
constexpr T ctSqrt(T x)
{
  return ctSqrt<T>(x, 0, x / 2 + 1);
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

template <typename T>
constexpr T ctSqrt(T x)
{
  return ctSqrt<T>(x, 0, x / 2 + 1);
}

#endif

template <typename T>
inline T isqrt(T x)
{
  T s = (T) std::sqrt((double) x);

  // By using constexpr for the sqrt_max variable type it
  // is guaranteed that ctSqrt() is evaluated at compile
  // time. Compilation will fail if the compiler fails to
  // evaluate ctSqrt() at compile time. This is great,
  // ctSqrt() must be evaluated at compile time otherwise
  // the runtime complexity of isqrt(x) would deteriorate
  // from O(1) to O(log2(x)).
  //
  // If sqrt_max were declared without constexpr then the
  // compiler would be free to compute ctSqrt() either at
  // compile time or at run time e.g. GCC-11 computes
  // ctSqrt(MAX_INT128) at compile time whereas Clang-12
  // computes ctSqrt(MAX_INT128) at run time even at -O2.
  //
  // C++20 fixed this annoying issue by adding consteval
  // to C++. Hence if the compiler supports C++20 ctSqrt()
  // is defined as consteval instead of constexpr. Hence
  // using C++20 ctSqrt() will be evaluated at compile
  // time in all cases i.e. even if sqrt_max were declared
  // without constexpr.
  //
  constexpr T sqrt_max = ctSqrt(std::numeric_limits<T>::max());
  T r = std::min(s, sqrt_max);

  // In my tests the first corrections were needed above
  // 10^22 where the results were off by 1. Above 10^32 the
  // first results occurred that were off by > 1. Since
  // primecount only supports numbers up to 10^31 this is
  // not an issue for us.
  if (r * r > x)
  {
    do { r--; }
    while (r * r > x);
  }
  // Same as (r + 1)^2 < x but overflow safe
  else if (r * 2 < x - r * r)
  {
    do { r++; }
    while (r * 2 < x - r * r);
  }

  return r;
}

} // namespace

#endif
