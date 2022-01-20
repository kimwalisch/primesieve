///
/// @file  intrinsics.hpp
/// @brief Wrappers for compiler intrinsics.
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef INTRINSICS_HPP
#define INTRINSICS_HPP

#include <stdint.h>
#include <cassert>

#if !defined(__has_builtin)
  #define __has_builtin(x) 0
#endif

#if !defined(__has_include)
  #define __has_include(x) 0
#endif

// GCC & Clang
#if defined(__GNUC__) || \
    __has_builtin(__builtin_popcountl)

namespace {

inline int popcnt64(uint64_t x)
{
#if __cplusplus >= 201703L
  if constexpr(sizeof(int) >= sizeof(uint64_t))
    return __builtin_popcount(x);
  else if constexpr(sizeof(long) >= sizeof(uint64_t))
    return __builtin_popcountl(x);
  else if constexpr(sizeof(long long) >= sizeof(uint64_t))
    return __builtin_popcountll(x);
#else
    return __builtin_popcountll(x);
#endif
}

} // namespace

#elif __cplusplus >= 202002L && \
      __has_include(<bit>)

#include <bit>
#define popcnt64(x) std::popcount(x)

#else

namespace {

/// This uses fewer arithmetic operations than any other known
/// implementation on machines with fast multiplication.
/// It uses 12 arithmetic operations, one of which is a multiply.
/// https://en.wikipedia.org/wiki/Hamming_weight#Efficient_implementation
///
inline int popcnt64(uint64_t x)
{
  uint64_t m1 = 0x5555555555555555ull;
  uint64_t m2 = 0x3333333333333333ull;
  uint64_t m4 = 0x0F0F0F0F0F0F0F0Full;
  uint64_t h01 = 0x0101010101010101ull;

  x -= (x >> 1) & m1;
  x = (x & m2) + ((x >> 2) & m2);
  x = (x + (x >> 4)) & m4;

  return (int) ((x * h01) >> 56);
}

} // namespace

#endif

// In 2022 std::countr_zero() is known to have very bad performance
// on these CPU architectures. Therefore don't use it.
#if (defined(__GNUC__) || \
     defined(__clang__)) && \
     defined(__x86_64__) && \
    !defined(__BMI__)
  #define DONT_USE_COUNTR_ZERO
#elif defined(_MSC_VER) && \
      defined(_M_X64) && \
     !defined(__AVX2__)
  #define DONT_USE_COUNTR_ZERO
#endif

// std::countr_zero(0) is well defined, unlike __builtin_ctzll(0)
// which causes undefined behavior. However, in 2022
// std::countr_zero() causes performance issues in many cases. On x64
// CPUs std::countr_zero() avoids undefined behavior by checking if
// the input number is 0 before executing BSF or by checking if the
// CPU supports TZCNT. GCC even adds zero checks on ARM64 where it is
// not needed. Until these issues are fixed we use inline assembly
// for the most important CPU architectures.

#if (defined(__GNUC__) || \
     defined(__clang__)) && \
     defined(__x86_64__) && \
     defined(__BMI__)

#define HAS_CTZ64
#define CTZ64_SUPPORTS_ZERO

namespace {

inline uint64_t ctz64(uint64_t x)
{
  // No undefined behavior, tzcnt(0) = 64
  __asm__("tzcnt %1, %0" : "=r"(x) : "r"(x));
  return x;
}

} // namespace

#elif (defined(__GNUC__) || \
       defined(__clang__)) && \
       defined(__aarch64__)

#define HAS_CTZ64
#define CTZ64_SUPPORTS_ZERO

namespace {

inline uint64_t ctz64(uint64_t x)
{
  // No undefined behavior, clz(0) = 64.
  // ARM64 has no CTZ instruction, we have to emulate it.
  __asm__("rbit %0, %1 \n\t"
          "clz %0, %0  \n\t"
          : "=r" (x)
          : "r" (x));

    return x;
}

} // namespace

#elif __cplusplus >= 202002L && \
      __has_include(<bit>) && \
      !defined(DONT_USE_COUNTR_ZERO)

#include <bit>

#define HAS_CTZ64
#define CTZ64_SUPPORTS_ZERO

// No undefined behavior, std::countr_zero(0) = 64
#define ctz64(x) std::countr_zero(x)

#elif defined(__GNUC__) || \
      __has_builtin(__builtin_ctzl)

#define HAS_CTZ64

namespace {

inline int ctz64(uint64_t x)
{
  // __builtin_ctz(0) is undefined behavior
  assert(x != 0);

#if __cplusplus >= 201703L
  if constexpr(sizeof(int) >= sizeof(uint64_t))
    return __builtin_ctz(x);
  else if constexpr(sizeof(long) >= sizeof(uint64_t))
    return __builtin_ctzl(x);
  else if constexpr(sizeof(long long) >= sizeof(uint64_t))
    return __builtin_ctzll(x);
#else
    return __builtin_ctzll(x);
#endif
}

} // namespace

#endif

#endif // INTRINSICS_HPP
