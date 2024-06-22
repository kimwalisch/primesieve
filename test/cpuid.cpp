///
/// @file   cpuid.cpp
/// @brief  Test CPUID code on x86 and x64 CPUs.
///
/// Copyright (C) 2024 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#if defined(ENABLE_MULTIARCH_x86_POPCNT)
  #include <primesieve/cpu_supports_popcnt.hpp>
#endif

#include <iostream>

int main()
{
#if defined(__x86_64__) || \
    defined(__i386__) || \
    defined(_M_X64) || \
    defined(_M_IX86)

  #if defined(__POPCNT__) && defined(ENABLE_MULTIARCH_x86_POPCNT)
    std::cerr << "Error: ENABLE_MULTIARCH_x86_POPCNT must not be defined if __POPCNT__ is defined!" << std::endl;
  #endif

  #if defined(_MSC_VER) && \
     !defined(__POPCNT__)

    #if defined(__AVX__) && defined(ENABLE_MULTIARCH_x86_POPCNT)
      std::cerr << "Error: ENABLE_MULTIARCH_x86_POPCNT must not be defined if __AVX__ is defined!" << std::endl;
    #endif
    #if defined(__AVX2__) && defined(ENABLE_MULTIARCH_x86_POPCNT)
      std::cerr << "Error: ENABLE_MULTIARCH_x86_POPCNT must not be defined if __AVX2__ is defined!" << std::endl;
    #endif
  #endif

  #if defined(ENABLE_MULTIARCH_x86_POPCNT)
    std::cout << "CPU supports POPCNT: " << (cpu_supports_popcnt ? "yes" : "no") << std::endl;
  #endif

#endif

  std::cout << std::endl;
  std::cout << "All tests passed successfully!" << std::endl;

  return 0;
}
