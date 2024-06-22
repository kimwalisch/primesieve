///
/// @file   ctz.hpp
/// @brief  Count the number of trailing zeros.
///
/// Copyright (C) 2024 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef CTZ_HPP
#define CTZ_HPP

#include "macros.hpp"
#include <stdint.h>

// GCC/Clang & MSVC
#if defined(__x86_64__) || \
    defined(_M_X64)
  #define IS_X64
#endif

// On x64 CPUs:
// GCC & Clang enable TZCNT with -mbmi.
// MSVC enables TZCNT with /arch:AVX2 or later.
#if defined(__BMI__) || \
   (defined(_MSC_VER) && defined(__AVX2__))
  #define HAS_TZCNT
#endif

#if (defined(__GNUC__) || \
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

#elif __cplusplus >= 202002L && \
      __has_include(<bit>) && \
     (!defined(IS_X64) || defined(HAS_TZCNT))

#include <bit>

// No undefined behavior, std::countr_zero(0) = 64
#define CTZ64_SUPPORTS_ZERO
#define HAS_CTZ64

namespace {

inline int ctz64(uint64_t x)
{
  // In 2022 std::countr_zero(x) generates good assembly for
  // most compilers & CPU architectures, except for:
  // 1) GCC & Clang on x64 without __BMI__.
  // 2) MSVC on x64 without __AVX2__.
  // Hence on x64 CPUs we only use std::countr_zero(x) if
  // the compiler generates the TZCNT instruction.
  return std::countr_zero(x);
}

} // namespace

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

#endif // CTZ_HPP
