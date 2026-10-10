///
/// @file  cpu_arch_macros.hpp
/// @brief Enable the fastest SIMD instruction set supported by the
///        compiler flags. Runtime dispatching (ENABLE_MULTIARCH_*)
///        is only kept for instruction sets that are even faster.
///
/// Copyright (C) 2026 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef CPU_ARCH_MACROS_HPP
#define CPU_ARCH_MACROS_HPP

#include "macros.hpp"

// Our ARM NEON algorithms require a 64-bit ARM CPU,
// on 32-bit ARM we use the portable default algorithms.
#if (defined(__aarch64__) || \
     defined(_M_ARM64)) && \
    __has_include(<arm_neon.h>)
  #define ENABLE_ARM_NEON
#endif

// PrimeGenerator.hpp and PreSieve.cpp disable AVX2
// runtime dispatching under different conditions,
// hence each of them uses its own AVX2 multiarch macro.
// PreSieve.cpp: ENABLE_MULTIARCH_AVX2
// PrimeGenerator.hpp: ENABLE_MULTIARCH_AVX2_BMI2
#if defined(ENABLE_MULTIARCH_AVX2)
  #define ENABLE_MULTIARCH_AVX2_BMI2
#endif

// PrimeGenerator.hpp SIMD algorithms
#if defined(__AVX512F__) && \
    defined(__AVX512VBMI__) && \
    defined(__AVX512VBMI2__) && \
    defined(__POPCNT__) && \
    !defined(__i386__) && \
    __has_include(<immintrin.h>)
  #define ENABLE_AVX512_VBMI2
  #undef ENABLE_MULTIARCH_AVX512_VBMI2
  #undef ENABLE_MULTIARCH_AVX2_BMI2
#elif defined(__AVX2__) && \
      defined(__BMI2__) && \
      defined(__POPCNT__) && \
      !defined(__i386__) && \
      __has_include(<immintrin.h>)
  #define ENABLE_AVX2_BMI2
  #undef ENABLE_MULTIARCH_AVX2_BMI2
#endif

// PreSieve.cpp SIMD algorithms
#if defined(__AVX512F__) && \
    defined(__AVX512BW__) && \
    __has_include(<immintrin.h>)
  #define ENABLE_AVX512_BW
  #undef ENABLE_MULTIARCH_AVX512_BW
  #undef ENABLE_MULTIARCH_AVX2
#elif defined(__AVX2__) && \
      __has_include(<immintrin.h>)
  #define ENABLE_AVX2
  #undef ENABLE_MULTIARCH_AVX2
#elif defined(__ARM_FEATURE_SVE) && \
      __has_include(<arm_sve.h>)
  #define ENABLE_ARM_SVE
  #undef ENABLE_MULTIARCH_ARM_SVE
#endif

#endif
