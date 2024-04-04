///
/// @file  CPUID.hpp
/// @brief POPCNT detection fo x86 and x86-64 CPUs.
///
/// Copyright (C) 2024 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef CPUID_HPP
#define CPUID_HPP

// Enable on x86 and x86-64 CPUs
#if defined(__x86_64__) || \
    defined(__i386__) || \
    defined(_M_X64) || \
    defined(_M_IX86)

// Both GCC and Clang (even Clang on Windows) define the __POPCNT__
// macro if the user compiles with -mpopcnt. The __POPCNT__
// macro is even defined if the user compiles with other flags
// such as -mavx or -march=native.
#if defined(__POPCNT__)
  #define HAS_POPCNT
// The MSVC compiler does not support a POPCNT macro, but if the user
// compiles with e.g. /arch:AVX or /arch:AVX512 then MSVC defines
// the __AVX__ macro and POPCNT is also supported.
#elif defined(_MSC_VER) && defined(__AVX__)
  #define HAS_POPCNT
#endif

#if defined(_MSC_VER)
  #include <intrin.h>
#endif

namespace {

void run_CPUID(int eax, int ecx, int* abcd)
{
#if defined(_MSC_VER)
  __cpuidex(abcd, eax, ecx);
#else
  int ebx = 0;
  int edx = 0;

  #if defined(__i386__) && \
      defined(__PIC__)
    /* in case of PIC under 32-bit EBX cannot be clobbered */
    __asm__ ("movl %%ebx, %%edi;"
             "cpuid;"
             "xchgl %%ebx, %%edi;"
             : "=D" (ebx),
               "+a" (eax),
               "+c" (ecx),
               "=d" (edx));
  #else
    __asm__ ("cpuid;"
             : "+b" (ebx),
               "+a" (eax),
               "+c" (ecx),
               "=d" (edx));
  #endif

  abcd[0] = eax;
  abcd[1] = ebx;
  abcd[2] = ecx;
  abcd[3] = edx;
#endif
}

#if !defined(HAS_POPCNT)
#define ENABLE_CPUID_POPCNT

bool run_CPUID_POPCNT()
{
  // %ecx POPCNT bit flag
  int bit_POPCNT = 1 << 23;
  int abcd[4];

  run_CPUID(1, 0, abcd);
  return (abcd[2] & bit_POPCNT) == bit_POPCNT;
}

/// Initialized at startup
const bool HAS_CPUID_POPCNT = run_CPUID_POPCNT();

#endif // ENABLE_CPUID_POPCNT

} // namespace

#endif // x86 CPU

#endif
