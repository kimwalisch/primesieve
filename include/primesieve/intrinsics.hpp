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

#include "macros.hpp"
#include <stdint.h>

#if !defined(__has_include)
  #define __has_include(x) 0
#endif

#if __cplusplus >= 202002L && \
    __has_include(<bit>)
  #include <bit>
  #define HAS_CPP20_BIT_HEADER
#endif

#if !defined(__has_builtin)
  #define __has_builtin(x) 0
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

#elif defined(HAS_CPP20_BIT_HEADER)

namespace {

inline int popcnt64(uint64_t x)
{
  return std::popcount(x);
}

} // namespace

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

// GCC/Clang & MSVC
#if defined(__x86_64__) || \
    defined(_M_X64)
  #define IS_X64
#endif

// On x64 CPUs:
// GCC & Clang enable TZCNT with -mbmi.
// MSVC enables TZCNT with /arch:AVX2 or /arch:AVX512.
#if defined(__BMI__) || \
   (defined(_MSC_VER) && (defined(__AVX2__) || defined(__AVX512__)))
  #define HAS_TZCNT
#endif

// In 2022 std::countr_zero(x) generates good assembly for
// most compilers & CPU architectures, except for:
// 1) GCC & Clang on x64 without __BMI__.
// 2) MSVC on x64 without __AVX2__.
// Hence on x64 CPUs we only use std::countr_zero(x) if
// the compiler generates the TZCNT instruction.
#if defined(HAS_CPP20_BIT_HEADER) && \
   (defined(HAS_TZCNT) || !defined(IS_X64))

#define HAS_CTZ64
#define CTZ64_SUPPORTS_ZERO

namespace {

inline int ctz64(uint64_t x)
{
  // No undefined behavior, std::countr_zero(0) = 64
  return std::countr_zero(x);
}

} // namespace

#elif (defined(__GNUC__) || \
       defined(__clang__)) && \
       defined(__x86_64__)

#define HAS_CTZ64
#define CTZ64_SUPPORTS_ZERO

namespace {

inline uint64_t ctz64(uint64_t x)
{
#if defined(HAS_TZCNT)
  // No undefined behavior, TZCNT(0) = 64
  __asm__("tzcnt %1, %0" : "=r"(x) : "r"(x));
  return x;
#else
  // REP BSF uses the TZCNT instruction on x64 CPUs with the BMI1
  // instruction set (>= 2013) and the BSF instruction on older x64
  // CPUs. BSF(0) is undefined behavior, it leaves the destination
  // register unmodified. Fortunately, it is possible to avoid this
  // undefined behavior by always setting the destination register
  // to the same value before executing BSF(0). This works on all
  // AMD & Intel CPUs since the i586 (from 1993), the Linux kernel
  // also relies on this behavior, see this Linux commit:
  // https://github.com/torvalds/linux/commit/ca3d30cc02f780f68771087040ce935add6ba2b7
  //
  // The constraint "0" for input operand 1 says that it must occupy
  // the same location as output operand 0. Hence the assembly below
  // uses the same input & output register. This ensures that
  // BSF(0) = 0, hence there is no undefined behavior. However, you
  // cannot rely on ctz64(0) = 0 since TZCNT(0) = 64.
  __asm__("rep bsf %1, %0" : "=r"(x) : "0"(x));
  ASSERT(x <= 64);
  return x;
#endif
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
  // No undefined behavior, CTZ(0) = 64.
  // ARM64 has no CTZ instruction, we have to emulate it.
  __asm__("rbit %0, %1 \n\t"
          "clz %0, %0  \n\t"
          : "=r" (x)
          : "r" (x));

  return x;
}

} // namespace

#elif defined(_MSC_VER) && \
      defined(HAS_TZCNT) && \
      __has_include(<immintrin.h>)

#include <immintrin.h>
#define HAS_CTZ64
#define CTZ64_SUPPORTS_ZERO

// This allows us to generate the TZCNT instruction for MSVC
// without C++20 support, hence without std::countr_zero(x).
// No undefined behavior, _tzcnt_u64(0) = 64.
#define ctz64(x) _tzcnt_u64(x)

#elif defined(__GNUC__) || \
      __has_builtin(__builtin_ctzl)

#define HAS_CTZ64

namespace {

inline int ctz64(uint64_t x)
{
  // __builtin_ctz(0) is undefined behavior,
  // we don't define CTZ64_SUPPORTS_ZERO.
  ASSERT(x != 0);

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
