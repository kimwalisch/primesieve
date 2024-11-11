///
/// @file  cpu_supports_avx512_bw.hpp
/// @brief Detect if the x86 CPU supports AVX512 BW.
///
/// Copyright (C) 2024 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef CPU_SUPPORTS_AVX512_BW_HPP
#define CPU_SUPPORTS_AVX512_BW_HPP

namespace primesieve {

bool has_cpuid_avx512_bw();

} // namespace

namespace {

/// Initialized at startup
const bool cpu_supports_avx512_bw = primesieve::has_cpuid_avx512_bw();

} // namespace

#endif
