///
/// @file   sve.cpp
/// @brief  Check if the CPU and OS support the SVE instruction set.
///         Compiling and linking of sve.cpp is tested by the CMake
///         build system using multiarch_sve_arm.cmake.
///
///         In order to generate optimal code, we need to be able to
///         check if the ARM CPU supports the SVE instruction set
///         in a global initializer when the program is loaded.
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

#if defined(_WIN32)

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
#include <errno.h>

namespace primesieve {

bool has_arm_sve()
{
  errno = 0;

  // getauxval() is supported by glibc >= 2.16 (since 2012),
  // musl libc >= 1.1.0 (2014) and Android's bionic libc (2010).
  // We check using CMake (multiarch_sve_arm.cmake) if
  // sve.cpp (and getauxval()) compiles and links correctly.
  unsigned long hwcaps = getauxval(AT_HWCAP);

  if (errno != 0)
    return false;

  // Check if the Linux kernel and the CPU support
  // the ARM SVE instruction set.
  if (hwcaps & HWCAP_SVE)
    return true;
  else
    return false;
}

} // namespace

#else

#ifndef __has_builtin
  #define __has_builtin(x) 0
#endif

#if __has_builtin(__builtin_cpu_init) && \
    __has_builtin(__builtin_cpu_supports)

namespace primesieve {

bool has_arm_sve()
{
  // Since __builtin_cpu_init() and __builtin_cpu_supports() are
  // currently (2025) not yet supported for ARM64 CPUs by both
  // GCC and Clang, we only use them as a fallback option if
  // none of the other more reliable methods work.
  __builtin_cpu_init();
  if (__builtin_cpu_supports("sve"))
    return true;
  else
    return false;
}

} // namespace

#endif
