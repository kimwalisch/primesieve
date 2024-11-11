///
/// @file  cpu_supports_arm_sve.hpp
///        Check if the CPU supports the ARM SVE instruction set.
///
/// Copyright (C) 2024 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef CPU_SUPPORTS_ARM_SVE_HPP
#define CPU_SUPPORTS_ARM_SVE_HPP

#include "macros.hpp"

#if __has_builtin(__builtin_cpu_supports)

namespace {

/// Initialized at startup
const bool cpu_supports_sve = __builtin_cpu_supports("sve");

} // namespace

#endif // __builtin_cpu_supports

#endif
