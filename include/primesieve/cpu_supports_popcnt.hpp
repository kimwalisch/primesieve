///
/// @file  cpu_supports_popcnt.hpp
/// @brief POPCNT detection fo x86 and x86-64 CPUs.
///
/// Copyright (C) 2024 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef CPU_SUPPORTS_POPCNT_HPP
#define CPU_SUPPORTS_POPCNT_HPP

namespace primesieve {

bool has_cpuid_popcnt();

} // namespace

namespace {

/// Initialized at startup
const bool cpu_supports_popcnt = primesieve::has_cpuid_popcnt();

} // namespace

#endif
