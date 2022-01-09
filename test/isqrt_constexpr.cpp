///
/// @file   isqrt_constexpr.cpp
/// @brief  Test compile time square root function.
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/pmath.hpp>

#include <stdint.h>
#include <limits>
#include <iostream>

#if defined(BAD_ISQRT)

/// The following compile time integer square root function
/// has a recursion depth of O(sqrt(n)). This is very bad, the
/// stack will explose if you try to compute the square root
/// of a number > 10^9. Furthermore constexpr recursion depth
/// is limited by the compiler even more e.g. both GCC and
/// Clang currently limit constexpr recursion depth to 512.
///
/// The nasty thing is that GCC and Clang start calculating
/// constexpr functions at compile time but once the function
/// reaches a recursion depth > 512, GCC and Clang will stop
/// the compile time calculation and instead compute the
/// function at run-time.
///
/// Because of this you cannot easily tell if your constexpr
/// function was evaluated at compile time or at run-time.
/// But you can use static_assert to test whether your constexpr
/// function is calculated at compile time or at runtime.
/// The code below will not compile with the compiler error
/// message telling you the maximum recursion depth has been
/// exceeded.

template <typename T>
constexpr T bad_isqrt(T sq, T dlt, T value)
{
  return sq > value ? (dlt >> 1) - 1
      : bad_isqrt<T>(sq + dlt, dlt + 2, value);
}

template <typename T>
constexpr T bad_isqrt(T value)
{
  return bad_isqrt<T>(1, 3, value);
}

static_assert(bad_isqrt(100000000) == 10000, "bad_isqrt(10^8) failed!");

#endif

int main()
{
  static_assert(ctSqrt(0) == 0, "ctSqrt(0) failed!");
  static_assert(ctSqrt(1) == 1, "ctSqrt(1) failed!");
  static_assert(ctSqrt(2) == 1, "ctSqrt(2) failed!");
  static_assert(ctSqrt(3) == 1, "ctSqrt(3) failed!");
  static_assert(ctSqrt(4) == 2, "ctSqrt(4) failed!");
  static_assert(ctSqrt(5) == 2, "ctSqrt(5) failed!");
  static_assert(ctSqrt(6) == 2, "ctSqrt(6) failed!");
  static_assert(ctSqrt(7) == 2, "ctSqrt(7) failed!");
  static_assert(ctSqrt(8) == 2, "ctSqrt(8) failed!");
  static_assert(ctSqrt(9) == 3, "ctSqrt(9) failed!");
  static_assert(ctSqrt(10) == 3, "ctSqrt(10) failed!");
  static_assert(ctSqrt(11) == 3, "ctSqrt(11) failed!");
  static_assert(ctSqrt(12) == 3, "ctSqrt(12) failed!");
  static_assert(ctSqrt(13) == 3, "ctSqrt(13) failed!");
  static_assert(ctSqrt(14) == 3, "ctSqrt(14) failed!");
  static_assert(ctSqrt(15) == 3, "ctSqrt(15) failed!");
  static_assert(ctSqrt(16) == 4, "ctSqrt(16) failed!");
  static_assert(ctSqrt(17) == 4, "ctSqrt(17) failed!");
  static_assert(ctSqrt(18) == 4, "ctSqrt(18) failed!");
  static_assert(ctSqrt(19) == 4, "ctSqrt(19) failed!");
  static_assert(ctSqrt(20) == 4, "ctSqrt(20) failed!");
  static_assert(ctSqrt(21) == 4, "ctSqrt(21) failed!");
  static_assert(ctSqrt(22) == 4, "ctSqrt(22) failed!");
  static_assert(ctSqrt(23) == 4, "ctSqrt(23) failed!");
  static_assert(ctSqrt(24) == 4, "ctSqrt(24) failed!");
  static_assert(ctSqrt(25) == 5, "ctSqrt(25) failed!");
  static_assert(ctSqrt(26) == 5, "ctSqrt(26) failed!");
  static_assert(ctSqrt(27) == 5, "ctSqrt(27) failed!");
  static_assert(ctSqrt(28) == 5, "ctSqrt(28) failed!");
  static_assert(ctSqrt(29) == 5, "ctSqrt(29) failed!");
  static_assert(ctSqrt(30) == 5, "ctSqrt(30) failed!");
  static_assert(ctSqrt(31) == 5, "ctSqrt(31) failed!");
  static_assert(ctSqrt(32) == 5, "ctSqrt(32) failed!");
  static_assert(ctSqrt(33) == 5, "ctSqrt(33) failed!");
  static_assert(ctSqrt(34) == 5, "ctSqrt(34) failed!");
  static_assert(ctSqrt(35) == 5, "ctSqrt(35) failed!");
  static_assert(ctSqrt(36) == 6, "ctSqrt(36) failed!");
  static_assert(ctSqrt(37) == 6, "ctSqrt(37) failed!");
  static_assert(ctSqrt(38) == 6, "ctSqrt(38) failed!");
  static_assert(ctSqrt(39) == 6, "ctSqrt(39) failed!");

  static_assert(ctSqrt(9223372037000249999ull) == 3037000499ull, "ctSqrt(3037000500^2-1) failed!");
  static_assert(ctSqrt(9223372037000250000ull) == 3037000500ull, "ctSqrt(3037000500^2) failed!");
  static_assert(ctSqrt(9223372037000250001ull) == 3037000500ull, "ctSqrt(3037000500^2+1) failed!");

  static_assert(ctSqrt(std::numeric_limits<int8_t>::max()) == 11, "ctSqrt(2^7-1) failed!");
  static_assert(ctSqrt(std::numeric_limits<uint8_t>::max()) == 15, "ctSqrt(2^8-1) failed!");
  static_assert(ctSqrt(std::numeric_limits<int16_t>::max()) == 181, "ctSqrt(2^15-1) failed!");
  static_assert(ctSqrt(std::numeric_limits<uint16_t>::max()) == 255, "ctSqrt(2^16-1) failed!");
  static_assert(ctSqrt(std::numeric_limits<int32_t>::max()) == 46340, "ctSqrt(2^31-1) failed!");
  static_assert(ctSqrt(std::numeric_limits<uint32_t>::max()) == 65535, "ctSqrt(2^32-1) failed!");
  static_assert(ctSqrt(std::numeric_limits<int64_t>::max()) == 3037000499ll, "ctSqrt(2^63-1) failed!");
  static_assert(ctSqrt(std::numeric_limits<uint64_t>::max()) == 4294967295ull, "ctSqrt(2^64-1) failed!");

  std::cout << "All tests passed successfully!" << std::endl;

  return 0;
}
