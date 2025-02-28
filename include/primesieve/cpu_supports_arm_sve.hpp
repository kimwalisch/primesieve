///
/// @file  cpu_supports_arm_sve.hpp
///        Check if the CPU supports the ARM SVE instruction set.
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef CPU_SUPPORTS_ARM_SVE_HPP
#define CPU_SUPPORTS_ARM_SVE_HPP

namespace primesieve {

bool has_arm_sve();

} // namespace

namespace {

/// Initialized at startup
const bool cpu_supports_sve = primesieve::has_arm_sve();

} // namespace

#endif
