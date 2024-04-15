///
/// @file   intrinsics.hpp
/// @brief  Wrappers for compiler intrinsics.
///
/// Copyright (C) 2024 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef INTRINSICS_HPP
#define INTRINSICS_HPP

#include "cpu_supports_popcnt.hpp"
#include "macros.hpp"

#include <stdint.h>

namespace {

/// This uses fewer arithmetic operations than any other known
/// implementation on machines with fast multiplication.
/// It uses 12 arithmetic operations, one of which is a multiply.
/// http://en.wikipedia.org/wiki/Hamming_weight#Efficient_implementation
///
inline uint64_t popcnt64_bitwise(uint64_t x)
{
  uint64_t m1 = 0x5555555555555555ll;
  uint64_t m2 = 0x3333333333333333ll;
  uint64_t m4 = 0x0F0F0F0F0F0F0F0Fll;
  uint64_t h01 = 0x0101010101010101ll;

  x -= (x >> 1) & m1;
  x = (x & m2) + ((x >> 2) & m2);
  x = (x + (x >> 4)) & m4;

  return (x * h01) >> 56;
}

} // namespace

// GCC & Clang
#if defined(__GNUC__) || \
    __has_builtin(__builtin_popcountl)

// CPUID is only enabled on x86 and x86-64 CPUs
// if the user compiles without -mpopcnt.
#if defined(ENABLE_CPUID_POPCNT)
#if defined(__x86_64__)

namespace {

inline uint64_t popcnt64(uint64_t x)
{
  // On my AMD EPYC 7642 CPU using GCC 12 this runtime
  // check incurs an overall overhead of about 1%.
  if_likely(cpu_supports_popcnt)
  {
    __asm__("popcnt %1, %0" : "=r"(x) : "r"(x));
    return x;
  }
  else
  {
    // On x86 and x64 CPUs when using the GCC compiler
    // __builtin_popcount*(x) is slow (not inlined function call)
    // when compiling without -mpopcnt. Therefore we avoid
    // using __builtin_popcount*(x) here.
    return popcnt64_bitwise(x);
  }
}

} // namespace

#elif defined(__i386__)

namespace {

inline uint64_t popcnt64(uint64_t x)
{
  if_likely(cpu_supports_popcnt)
  {
    uint32_t x0 = uint32_t(x);
    uint32_t x1 = uint32_t(x >> 32);
    __asm__("popcnt %1, %0" : "=r"(x0) : "r"(x0));
    __asm__("popcnt %1, %0" : "=r"(x1) : "r"(x1));
    return x0 + x1;
  }
  else
  {
    // On x86 and x64 CPUs when using the GCC compiler
    // __builtin_popcount*(x) is slow (not inlined function call)
    // when compiling without -mpopcnt. Therefore we avoid
    // using __builtin_popcount*(x) here.
    return popcnt64_bitwise(x);
  }
}

} // namespace

#endif // i386

#else // GCC & Clang (no CPUID, not x86)

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

#endif // GCC & Clang

#elif defined(_MSC_VER) && \
      defined(_M_X64) && \
      __has_include(<intrin.h>)

#include <intrin.h>

namespace {

inline uint64_t popcnt64(uint64_t x)
{
#if defined(HAS_POPCNT)
  return __popcnt64(x);
#elif defined(ENABLE_CPUID_POPCNT)
  if_likely(cpu_supports_popcnt)
    return __popcnt64(x);
  else
    return popcnt64_bitwise(x);
#else
  return popcnt64_bitwise(x);
#endif
}

} // namespace

#elif defined(_MSC_VER) && \
      defined(_M_IX86) && \
      __has_include(<intrin.h>)

#include <intrin.h>

namespace {

inline uint64_t popcnt64(uint64_t x)
{
#if defined(HAS_POPCNT)
  return __popcnt(uint32_t(x)) +
         __popcnt(uint32_t(x >> 32));
#elif defined(ENABLE_CPUID_POPCNT)
  if_likely(cpu_supports_popcnt)
    return __popcnt(uint32_t(x)) +
           __popcnt(uint32_t(x >> 32));
  else
    return popcnt64_bitwise(x);
#else
  return popcnt64_bitwise(x);
#endif
}

} // namespace

#elif #if __cplusplus >= 202002L && \
      __has_include(<bit>)

#include <bit>

namespace {

/// We only use the C++ standard library as a fallback if there
/// are no compiler intrinsics available for POPCNT.
/// Compiler intrinsics often generate faster assembly.
inline int popcnt64(uint64_t x)
{
  return std::popcount(x);
}

} // namespace

#else

namespace {

/// Portable (but slow) popcount algorithm
inline uint64_t popcnt64(uint64_t x)
{
  return popcnt64_bitwise(x);
}

} // namespace

#endif // popcnt64()

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
