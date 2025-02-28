///
/// @file   sve.cpp
/// @brief  Check if the CPU and OS support the SVE instruction set.
///         In order to generate optimal code, we need to be able to
///         check if the ARM CPU supports the SVE instruction set in
///         a global initializer when the program is loaded.
///
///         __builtin_cpu_supports() from Clang >= 19.0.0 does not
///         work when running in a global initializer. Usually the
///         workaround for this issue is to call __builtin_cpu_init()
///         before calling __builtin_cpu_supports(). However,
///         __builtin_cpu_init() is currently not supported on
///         ARM CPUs.
///
///         TODO: Add macOS support once Apple ARM CPUs support the
///               SVE instruction yet.
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef __has_builtin
  #define __has_builtin(x) 0
#endif

#if __has_builtin(__builtin_cpu_init) && \
    __has_builtin(__builtin_cpu_supports)

namespace primesieve {

bool has_arm_sve()
{
  __builtin_cpu_init();
  if (__builtin_cpu_supports("sve"))
    return true;
  else
    return false;
}

} // namespace

#elif defined(_WIN32)

#include <windows.h>

namespace primesieve {

bool has_arm_sve()
{
#if defined(PF_ARM_SVE_INSTRUCTIONS_AVAILABLE)
  return IsProcessorFeaturePresent(PF_ARM_SVE_INSTRUCTIONS_AVAILABLE);
#else
  return false;
#endif
}

} // namespace

#elif defined(__linux__) || \
      defined(__gnu_linux__) || \
      defined(__ANDROID__)

#include <sys/auxv.h>
#include <asm/hwcap.h>

namespace primesieve {

/// Check if the Linux kernel and the CPU support
/// the ARM SVE instruction set. 
bool has_arm_sve()
{
  unsigned long hwcaps = getauxval(AT_HWCAP);

  if (hwcaps & HWCAP_SVE)
    return true;
  else
    return false;
}

} // namespace

#endif
