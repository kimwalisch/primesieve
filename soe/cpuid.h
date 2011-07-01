/*
 * cpuid.h -- This file is part of primesieve
 *
 * Copyright (C) 2011 Kim Walisch, <kim.walisch@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */
 
 /** 
 * @file cpuid.h 
 * @brief Contains a portable implementation of cpuid for x86 and x64
 *        CPUs and macros/functions that make use of compiler
 *        intrinsics or built-in functions.
 */

#ifndef CPUID_H
#define CPUID_H

#include <climits>
#include <cassert>

#define bit_POPCNT (1 << 23)

#if defined(_MSC_VER) && (defined(_WIN32) || defined(_WIN64)) || \
    defined(__INTEL_COMPILER) && (defined(_WIN32) || defined(_WIN64))
#  define MSC_X86_COMPATIBLE
#  include <intrin.h> /* __cpuid(), _BitScanForward() */
#elif defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__)) || \
      defined(__SUNPRO_CC) && (defined(__i386) || defined(__x86_64))
#  define GCC_I386_COMPATIBLE
#  include "cpuidgcc460.h" /* gcc-4.6.0/gcc/config/i386/cpuid.h */
#endif

/**
 * @def POPCNT64(addr)
 * Count the number of 1 bits within the next 8 bytes of an array
 * using the SSE4.2 POPCNT instruction.
 * @pre Check if the CPU supports POPCNT with isPOPCNTSupported()
 * @pre addr must be aligned to an 8-byte boundary
 *
 * Successfully tested with:
 *
 *   Microsoft Visual Studio 2010 (WIN32, WIN64),
 *   GNU G++ 4.5 (x86, x64),
 *   Intel C++ Compiler 12.0 (x86, x64),
 *   Oracle Solaris Studio 12 (x64).
 */
#if defined(MSC_X86_COMPATIBLE) && defined(_WIN64)
#  include <nmmintrin.h> 
#  define POPCNT64(addr) static_cast<uint32_t> \
      (_mm_popcnt_u64(*reinterpret_cast<const uint64_t*> (addr)))
#elif defined(MSC_X86_COMPATIBLE) && defined(_WIN32)
#  include <nmmintrin.h>
#  define POPCNT64(addr) \
      (_mm_popcnt_u32(reinterpret_cast<const uint32_t*> (addr)[0]) + \
       _mm_popcnt_u32(reinterpret_cast<const uint32_t*> (addr)[1]))
#elif defined(__SUNPRO_CC) && defined(__x86_64)
#  include <nmmintrin.h>
#  define POPCNT64(addr) static_cast<uint32_t> \
      (_mm_popcnt_u64(*reinterpret_cast<const uint64_t*> (addr)))
#elif defined(__GNUC__) && defined(__x86_64__)
#  define POPCNT64(addr) static_cast<uint32_t> \
      (__builtin_popcountll(*reinterpret_cast<const uint64_t*> (addr)))
#elif defined(__GNUC__) && defined(__i386__) && INT_MAX >= 2147483647
#  define POPCNT64(addr) \
      (__builtin_popcount(reinterpret_cast<const uint32_t*> (addr)[0]) + \
       __builtin_popcount(reinterpret_cast<const uint32_t*> (addr)[1]))
#endif

/**
 * Portable implementation of cpuid for x86 and x64 CPUs.
 * @return 1 if the CPU supports the cpuid instruction.
 *         0 if the CPU does not support the cpuid instruction.
 *        -1 if the target architecture (or compiler) is not supported.
 *
 * Successfully tested with:
 *
 *   Microsoft Visual Studio 2010,
 *   GNU G++ 4.5,
 *   Apple G++ 4.2,
 *   LLVM 2.9,
 *   Intel C++ Compiler 12.0,
 *   AMD x86 Open64 Compiler Suite,
 *   Oracle Solaris Studio 12.
 */
inline int getCPUID(unsigned int __level, unsigned int *__eax,
    unsigned int *__ebx, unsigned int *__ecx, unsigned int *__edx) {
#if defined(MSC_X86_COMPATIBLE)
  int CPUInfo[4] = {*__eax, *__ebx, *__ecx, *__edx};
  __cpuid(CPUInfo, 0);
  // check if the CPU supports the cpuid instruction.
  if (CPUInfo[0] != 0) {
    __cpuid(CPUInfo, __level);
    *__eax = CPUInfo[0];
    *__ebx = CPUInfo[1];
    *__ecx = CPUInfo[2];
    *__edx = CPUInfo[3];
    return 1;
  }
  return 0;
#elif defined(GCC_I386_COMPATIBLE)
  // __get_cpuid() returns 1 if the CPU supports cpuid else 0
  return __get_cpuid(__level, __eax, __ebx, __ecx, __edx);
#else
  return -1;
#endif
}

/**
 * Check if the CPU supports the SSE4.2 POPCNT instruction using
 * cpuid.
 */
inline bool isPOPCNTSupported() {
  unsigned int info_type = 0x00000001;
  unsigned int ax, bx, cx, dx;
  if (getCPUID(info_type, &ax, &bx, &cx, &dx) == 1)
    return ((cx & bit_POPCNT) != 0);
  return false;
}

/**
 * Search the operand (v) for the least significant set bit (1 bit)
 * and return its position.
 * @pre v != 0
 */
inline uint32_t bitScanForward(uint32_t v) {
  assert(v != 0);
#if defined(MSC_X86_COMPATIBLE)
  unsigned long r;
  _BitScanForward(&r, static_cast<unsigned long> (v));
  return static_cast<uint32_t> (r);
#elif defined(__GNUC__) && INT_MAX >= 2147483647
  return static_cast<uint32_t> (__builtin_ctz(v));
#else
  // Count the consecutive zero bits (trailing) on the right with
  // multiply and lookup, code from "Bit Twiddling Hacks":
  // http://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightMultLookup
  static const uint32_t MultiplyDeBruijnBitPosition[32] = {
       0,  1, 28,  2, 29, 14, 24, 3,
      30, 22, 20, 15, 25, 17,  4, 8,
      31, 27, 13, 23, 21, 19, 16, 7,
      26, 12, 18,  6, 11,  5, 10, 9
  };
  const int32_t s = static_cast<int32_t> (v);
  return MultiplyDeBruijnBitPosition[((s & -s) * 0x077CB531U) >> 27];
#endif
}

#endif /* CPUID_H */
