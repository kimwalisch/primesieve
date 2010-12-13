/*
 * cpuid.h -- This file is part of primesieve
 *
 * Copyright (C) 2010 Kim Walisch, <kim.walisch@gmail.com>
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

#define bit_POPCNT	(1 << 23)

#if defined(_MSC_VER) || ((defined(_WIN32) || defined(_WIN64)) && defined(__INTEL_COMPILER))
#  define MSC_CPUID
#  define USE_POPCNT 
#  include <intrin.h> // __cpuid()
#elif defined(__GNUC__) || defined(__SUNPRO_CC)
#  define GCC_CPUID
#  define USE_POPCNT 
#  include "cpuid_gcc451.h" // cpuid.h of GCC 4.5.1
#endif

/**
 * @def POPCNT64(n)
 * Macro that takes advantage of the SSE 4.2 POPCNT instruction, works
 * with x86 and x64_86 CPUs.
 */
#if defined(USE_POPCNT)
#  define POPCNT64_x64(popcnt, addr, i) static_cast<uint32_t> (popcnt(*reinterpret_cast<const uint64_t*>(addr + i)))
#  define POPCNT64_x86(popcnt, addr, i) (popcnt(*reinterpret_cast<const uint32_t*>(addr + i)) + \
                                         popcnt(*reinterpret_cast<const uint32_t*>(addr + i + 4)))
#  if defined(MSC_CPUID)
#    include <nmmintrin.h> // _mm_popcnt_u32(), _mm_popcnt_u64()
#    if defined(_WIN64)
#      define POPCNT64(addr, i) POPCNT64_x64(_mm_popcnt_u64, addr, i)
#    else if defined(_WIN32)
#      define POPCNT64(addr, i) POPCNT64_x86(_mm_popcnt_u32, addr, i)
#    endif
#  elif defined(__SUNPRO_CC)
#    include <nmmintrin.h>
#    if definied(__x86_64)
#      define POPCNT64(addr, i) POPCNT64_x64(_mm_popcnt_u64, addr, i)
#    else if defined(__i386)
#      define POPCNT64(addr, i) POPCNT64_x86(_mm_popcnt_u32, addr, i)
#    endif
#  elif defined(__GNUC__)
#    if definied(__x86_64__)
#      define POPCNT64(addr, i) POPCNT64_x64(__builtin_popcountll, addr, i)
#    else if defined(__i386__)
#      define POPCNT64(addr, i) POPCNT64_x86(__builtin_popcount, addr, i)
#    endif
#  endif
#endif

namespace utils {
  /**
   * Portable implementation of CPUID, successfully tested with:
   * - MSVC 2008 & 2010,
   * - GNU GCC 4.5,
   * - Intel C++ Compiler 11.1,
   * - Sun Studio 12,
   * - AMD x86 Open64 Compiler Suite.
   * @return 1 if the CPU supports the cpuid instruction.
   *         0 if the CPU does not support the cpuid instruction.
   *        -1 if the compiler is not supported.
   */
  inline int getCPUID(unsigned int __level, unsigned int *__eax,
      unsigned int *__ebx, unsigned int *__ecx, unsigned int *__edx) {
#if defined(MSC_CPUID) // Microsoft Visual C++ compatible compilers
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
#elif defined(GCC_CPUID) // GNU GCC compatible compilers
    // __get_cpuid() returns 1 if the CPU supports cpuid else 0
    return __get_cpuid(__level, __eax, __ebx, __ecx, __edx);
#else
    return -1;
#endif
  }

  /**                      
   * @return true if the CPU supports the SSE 4.2 POPCNT instruction
   * else false.
   * Microsoft __popcnt documentation: 
   * http://msdn.microsoft.com/en-en/library/bb385231.aspx
   */
  inline bool isPOPCNTSupported() {
    unsigned int info_type = 0x00000001;
    unsigned int ax, bx, cx, dx;
    if (getCPUID(info_type, &ax, &bx, &cx, &dx) == 1)
      return ((cx & bit_POPCNT) != 0);
    return false;
  }
}

#endif /* CPUID_H */

