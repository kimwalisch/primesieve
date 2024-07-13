///
/// @file  popcnt.hpp
/// @brief Functions to count the number of 1 bits inside
///        a 64-bit variable.
///
/// Copyright (C) 2024 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef POPCNT_HPP
#define POPCNT_HPP

#include "macros.hpp"
#include <stdint.h>

#if defined(ENABLE_MULTIARCH_x86_POPCNT)
  #include "cpu_supports_popcnt.hpp"
#endif

// GCC & Clang
#if defined(__GNUC__) || \
    __has_builtin(__builtin_popcountl)

// CPUID is only enabled on x86 and x86-64 CPUs
// if the user compiles without -mpopcnt.
#if defined(ENABLE_MULTIARCH_x86_POPCNT)
#if defined(__x86_64__)

namespace {

/// This uses fewer arithmetic operations than any other known
/// implementation on machines with fast multiplication.
/// It uses 12 arithmetic operations, one of which is a multiply.
/// http://en.wikipedia.org/wiki/Hamming_weight#Efficient_implementation
///
NOINLINE uint64_t popcnt64_bitwise_noinline(uint64_t x)
{
  uint64_t m1 = 0x5555555555555555ull;
  uint64_t m2 = 0x3333333333333333ull;
  uint64_t m4 = 0x0F0F0F0F0F0F0F0Full;
  uint64_t h01 = 0x0101010101010101ull;

  x -= (x >> 1) & m1;
  x = (x & m2) + ((x >> 2) & m2);
  x = (x + (x >> 4)) & m4;

  return (x * h01) >> 56;
}

ALWAYS_INLINE uint64_t popcnt64(uint64_t x)
{
  // On my AMD EPYC 7642 CPU using GCC 12 this runtime
  // check incurs an overall overhead of about 1%.
  if_likely(cpu_supports_popcnt)
  {
    __asm__("popcnt %1, %0" : "=r"(x) : "r"(x));
    return x;
  }
  else
    return popcnt64_bitwise_noinline(x);
}

} // namespace

#elif defined(__i386__)

namespace {

/// This uses fewer arithmetic operations than any other known
/// implementation on machines with fast multiplication.
/// It uses 12 arithmetic operations, one of which is a multiply.
/// http://en.wikipedia.org/wiki/Hamming_weight#Efficient_implementation
///
NOINLINE uint64_t popcnt64_bitwise_noinline(uint64_t x)
{
  uint64_t m1 = 0x5555555555555555ull;
  uint64_t m2 = 0x3333333333333333ull;
  uint64_t m4 = 0x0F0F0F0F0F0F0F0Full;
  uint64_t h01 = 0x0101010101010101ull;

  x -= (x >> 1) & m1;
  x = (x & m2) + ((x >> 2) & m2);
  x = (x + (x >> 4)) & m4;

  return (x * h01) >> 56;
}

ALWAYS_INLINE uint64_t popcnt64(uint64_t x)
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
    return popcnt64_bitwise_noinline(x);
}

} // namespace

#endif // i386

#else // GCC & Clang (no CPUID, not x86)

namespace {

ALWAYS_INLINE uint64_t popcnt64(uint64_t x)
{
#if __cplusplus >= 201703L
  if constexpr(sizeof(int) >= sizeof(uint64_t))
    return (uint64_t) __builtin_popcount(x);
  else if constexpr(sizeof(long) >= sizeof(uint64_t))
    return (uint64_t) __builtin_popcountl(x);
  else if constexpr(sizeof(long long) >= sizeof(uint64_t))
    return (uint64_t) __builtin_popcountll(x);
#else
    return (uint64_t) __builtin_popcountll(x);
#endif
}

} // namespace

#endif // GCC & Clang

#elif defined(_MSC_VER) && \
      defined(_M_X64) && \
      __has_include(<intrin.h>)

#include <intrin.h>

namespace {

#if defined(__POPCNT__) || \
    defined(__AVX__)

ALWAYS_INLINE uint64_t popcnt64(uint64_t x)
{
  return __popcnt64(x);
}

#elif defined(ENABLE_MULTIARCH_x86_POPCNT)

/// This uses fewer arithmetic operations than any other known
/// implementation on machines with fast multiplication.
/// It uses 12 arithmetic operations, one of which is a multiply.
/// http://en.wikipedia.org/wiki/Hamming_weight#Efficient_implementation
///
NOINLINE uint64_t popcnt64_bitwise_noinline(uint64_t x)
{
  uint64_t m1 = 0x5555555555555555ull;
  uint64_t m2 = 0x3333333333333333ull;
  uint64_t m4 = 0x0F0F0F0F0F0F0F0Full;
  uint64_t h01 = 0x0101010101010101ull;

  x -= (x >> 1) & m1;
  x = (x & m2) + ((x >> 2) & m2);
  x = (x + (x >> 4)) & m4;

  return (x * h01) >> 56;
}

ALWAYS_INLINE uint64_t popcnt64(uint64_t x)
{
  if_likely(cpu_supports_popcnt)
    return __popcnt64(x);
  else
    return popcnt64_bitwise_noinline(x);
}

#else

/// This uses fewer arithmetic operations than any other known
/// implementation on machines with fast multiplication.
/// It uses 12 arithmetic operations, one of which is a multiply.
/// http://en.wikipedia.org/wiki/Hamming_weight#Efficient_implementation
///
ALWAYS_INLINE uint64_t popcnt64(uint64_t x)
{
  uint64_t m1 = 0x5555555555555555ull;
  uint64_t m2 = 0x3333333333333333ull;
  uint64_t m4 = 0x0F0F0F0F0F0F0F0Full;
  uint64_t h01 = 0x0101010101010101ull;

  x -= (x >> 1) & m1;
  x = (x & m2) + ((x >> 2) & m2);
  x = (x + (x >> 4)) & m4;

  return (x * h01) >> 56;
}

#endif

} // namespace

#elif defined(_MSC_VER) && \
      defined(_M_IX86) && \
      __has_include(<intrin.h>)

#include <intrin.h>

namespace {

#if defined(__POPCNT__) || \
    defined(__AVX__)

ALWAYS_INLINE uint64_t popcnt64(uint64_t x)
{
  return __popcnt(uint32_t(x)) +
         __popcnt(uint32_t(x >> 32));
}

#elif defined(ENABLE_MULTIARCH_x86_POPCNT)

/// This uses fewer arithmetic operations than any other known
/// implementation on machines with fast multiplication.
/// It uses 12 arithmetic operations, one of which is a multiply.
/// http://en.wikipedia.org/wiki/Hamming_weight#Efficient_implementation
///
NOINLINE uint64_t popcnt64_bitwise_noinline(uint64_t x)
{
  uint64_t m1 = 0x5555555555555555ull;
  uint64_t m2 = 0x3333333333333333ull;
  uint64_t m4 = 0x0F0F0F0F0F0F0F0Full;
  uint64_t h01 = 0x0101010101010101ull;

  x -= (x >> 1) & m1;
  x = (x & m2) + ((x >> 2) & m2);
  x = (x + (x >> 4)) & m4;

  return (x * h01) >> 56;
}

ALWAYS_INLINE uint64_t popcnt64(uint64_t x)
{
  if_likely(cpu_supports_popcnt)
    return __popcnt(uint32_t(x)) +
           __popcnt(uint32_t(x >> 32));
  else
    return popcnt64_bitwise_noinline(x);
}

#else

/// This uses fewer arithmetic operations than any other known
/// implementation on machines with fast multiplication.
/// It uses 12 arithmetic operations, one of which is a multiply.
/// http://en.wikipedia.org/wiki/Hamming_weight#Efficient_implementation
///
ALWAYS_INLINE uint64_t popcnt64(uint64_t x)
{
  uint64_t m1 = 0x5555555555555555ull;
  uint64_t m2 = 0x3333333333333333ull;
  uint64_t m4 = 0x0F0F0F0F0F0F0F0Full;
  uint64_t h01 = 0x0101010101010101ull;

  x -= (x >> 1) & m1;
  x = (x & m2) + ((x >> 2) & m2);
  x = (x + (x >> 4)) & m4;

  return (x * h01) >> 56;
}

#endif

} // namespace

#elif __cplusplus >= 202002L && \
      __has_include(<bit>)

#include <bit>

namespace {

/// We only use the C++ standard library as a fallback if there
/// are no compiler intrinsics available for POPCNT.
/// Compiler intrinsics often generate faster assembly.
ALWAYS_INLINE uint64_t popcnt64(uint64_t x)
{
  return std::popcount(x);
}

} // namespace

#else

namespace {

/// This uses fewer arithmetic operations than any other known
/// implementation on machines with fast multiplication.
/// It uses 12 arithmetic operations, one of which is a multiply.
/// http://en.wikipedia.org/wiki/Hamming_weight#Efficient_implementation
///
ALWAYS_INLINE uint64_t popcnt64(uint64_t x)
{
  uint64_t m1 = 0x5555555555555555ull;
  uint64_t m2 = 0x3333333333333333ull;
  uint64_t m4 = 0x0F0F0F0F0F0F0F0Full;
  uint64_t h01 = 0x0101010101010101ull;

  x -= (x >> 1) & m1;
  x = (x & m2) + ((x >> 2) & m2);
  x = (x + (x >> 4)) & m4;

  return (x * h01) >> 56;
}

} // namespace

#endif

#endif // POPCNT_HPP
