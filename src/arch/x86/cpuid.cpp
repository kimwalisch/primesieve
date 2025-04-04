///
/// @file  cpuid.cpp
/// @brief CPUID for x86 and x86-64 CPUs.
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <stdint.h>

#if defined(_MSC_VER)
  #include <intrin.h>
  #include <immintrin.h>
#endif

// CPUID bits documentation:
// https://en.wikipedia.org/wiki/CPUID

// %ebx bit flags
#define bit_AVX512F  (1 << 16)
#define bit_AVX512BW (1 << 30)

// %ecx bit flags
#define bit_AVX512VBMI  (1 << 1)
#define bit_AVX512VBMI2 (1 << 6)
#define bit_POPCNT      (1 << 23)

// xgetbv bit flags
#define XSTATE_SSE (1 << 1)
#define XSTATE_YMM (1 << 2)
#define XSTATE_ZMM (7 << 5)

namespace {

void run_cpuid(int eax, int ecx, int* abcd)
{
#if defined(_MSC_VER)
  __cpuidex(abcd, eax, ecx);
#else
  int ebx = 0;
  int edx = 0;

  #if defined(__i386__) && \
      defined(__PIC__)
    // In case of PIC under 32-bit EBX cannot be clobbered
    __asm__ __volatile__("movl %%ebx, %%edi;"
                         "cpuid;"
                         "xchgl %%ebx, %%edi;"
                         : "+a" (eax),
                           "=D" (ebx),
                           "+c" (ecx),
                           "=d" (edx));
  #else
    __asm__ __volatile__("cpuid"
                         : "+a" (eax),
                           "+b" (ebx),
                           "+c" (ecx),
                           "=d" (edx));
  #endif

  abcd[0] = eax;
  abcd[1] = ebx;
  abcd[2] = ecx;
  abcd[3] = edx;
#endif
}

// Get Value of Extended Control Register
uint64_t get_xcr0()
{
#if defined(_MSC_VER)
  return _xgetbv(0);
#else
  uint32_t eax;
  uint32_t edx;

  __asm__ __volatile__("xgetbv" : "=a"(eax), "=d"(edx) : "c"(0));
  return eax | (uint64_t(edx) << 32);
#endif
}

} // namespace

namespace primesieve {

bool has_cpuid_popcnt()
{
  int abcd[4];
  run_cpuid(1, 0, abcd);
  return (abcd[2] & bit_POPCNT) == bit_POPCNT;
}

bool has_cpuid_avx512_bw()
{
  int abcd[4];

  run_cpuid(1, 0, abcd);

  int osxsave_mask = (1 << 27);

  // Ensure OS supports extended processor state management
  if ((abcd[2] & osxsave_mask) != osxsave_mask)
    return false;

  uint64_t ymm_mask = XSTATE_SSE | XSTATE_YMM;
  uint64_t zmm_mask = XSTATE_SSE | XSTATE_YMM | XSTATE_ZMM;
  uint64_t xcr0 = get_xcr0();

  // Check AVX OS support
  if ((xcr0 & ymm_mask) != ymm_mask)
    return false;

  // Check AVX512 OS support
  if ((xcr0 & zmm_mask) != zmm_mask)
    return false;

  run_cpuid(7, 0, abcd);

  // presieve1_x86_avx512() requires AVX512F, AVX512BW
  return ((abcd[1] & bit_AVX512F) == bit_AVX512F &&
          (abcd[1] & bit_AVX512BW) == bit_AVX512BW);
}

bool has_cpuid_avx512_vbmi2()
{
  int abcd[4];

  run_cpuid(1, 0, abcd);

  int osxsave_mask = (1 << 27);

  // Ensure OS supports extended processor state management
  if ((abcd[2] & osxsave_mask) != osxsave_mask)
    return false;

  uint64_t ymm_mask = XSTATE_SSE | XSTATE_YMM;
  uint64_t zmm_mask = XSTATE_SSE | XSTATE_YMM | XSTATE_ZMM;
  uint64_t xcr0 = get_xcr0();

  // Check AVX OS support
  if ((xcr0 & ymm_mask) != ymm_mask)
    return false;

  // Check AVX512 OS support
  if ((xcr0 & zmm_mask) != zmm_mask)
    return false;

  run_cpuid(7, 0, abcd);

  // fillNextPrimes_x86_avx512() requires AVX512F, AVX512VBMI & AVX512VBMI2
  return ((abcd[1] & bit_AVX512F) == bit_AVX512F &&
          (abcd[2] & (bit_AVX512VBMI | bit_AVX512VBMI2)) == (bit_AVX512VBMI | bit_AVX512VBMI2));
}

} // namespace
