///
/// @file  cpu_supports_avx512_vbmi2.hpp
/// @brief Detect if the x86 CPU supports AVX512 VBMI2.
///
/// Copyright (C) 2024 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef CPU_SUPPORTS_AVX512_VBMI2_HPP
#define CPU_SUPPORTS_AVX512_VBMI2_HPP

namespace primesieve {

bool has_cpuid_avx512_vbmi2();

} // namespace

namespace {

/// Initialized at startup
const bool cpu_supports_avx512_vbmi2 = primesieve::has_cpuid_avx512_vbmi2();

} // namespace

#endif
