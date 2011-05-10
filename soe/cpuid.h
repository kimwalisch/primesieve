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

#ifndef CPUID_H
#define CPUID_H

#if (defined(_MSC_VER) || defined(__INTEL_COMPILER)) && (defined(_WIN32) || defined(_WIN64))
#  include <intrin.h> // __cpuid()
#  define MSC_X86_COMPATIBLE
#  define bit_POPCNT (1 << 23)
#elif defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__)) || \
      defined(__SUNPRO_CC) && (defined(__i386) || defined(__x86_64))
#  include "cpuidgcc460.h" // gcc-4.6.0/gcc/config/i386/cpuid.h
#  define GCC_I386_COMPATIBLE
#endif

/**
 * Portable implementation of CPUID for x86 and x64 CPUs.
 * Successfully tested with:
 *    Microsoft Visual Studio 2010,
 *    GNU G++ 4.5,
 *    Apple G++ 4.2,
 *    LLVM 2.9,
 *    Intel C++ Compiler 12.0,
 *    AMD x86 Open64 Compiler Suite,
 *    Oracle Solaris Studio 12.
 * @return 1 if the CPU supports the cpuid instruction.
 *         0 if the CPU does not support the cpuid instruction.
 *        -1 if the compiler is not supported.
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
 * @return true if the CPU supports the SSE 4.2 POPCNT instruction
 *         (and cpuid) else false.
 * @see http://en.wikipedia.org/wiki/SSE4#SSE4.2
 */
inline bool isPOPCNTSupported() {
  unsigned int info_type = 0x00000001;
  unsigned int ax, bx, cx, dx;
  if (getCPUID(info_type, &ax, &bx, &cx, &dx) == 1)
    return ((cx & bit_POPCNT) != 0);
  return false;
}

#endif /* CPUID_H */

