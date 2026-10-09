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

// Needed for __has_include
#include "macros.hpp"

#if defined(__AVX512F__) && \
    defined(__AVX512VBMI__) && \
    defined(__AVX512VBMI2__) && \
    __has_include(<immintrin.h>)
  #define ENABLE_AVX512_VBMI2
  #undef ENABLE_MULTIARCH_AVX512_VBMI2
  #undef ENABLE_MULTIARCH_AVX2
#elif defined(__AVX2__) && \
      defined(__BMI2__) && \
      defined(__POPCNT__) && \
      __has_include(<immintrin.h>)
  #define ENABLE_AVX2
  #undef ENABLE_MULTIARCH_AVX2
#elif defined(__aarch64__) && \
      __has_include(<arm_neon.h>)
  // Our ARM NEON code uses vmovl_high() and vaddw_high() which
  // are supported on 64-bit but not on 32-bit ARM CPUs.
  #define ENABLE_ARM_NEON
#endif

#endif
