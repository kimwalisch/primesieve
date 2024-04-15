///
/// @file   cpuid.cpp
/// @brief  Test CPUID code on x86 and x64 CPUs.
///
/// Copyright (C) 2024 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/cpu_supports_popcnt.hpp>
#include <iostream>

int main()
{
#if defined(__x86_64__) || \
    defined(__i386__) || \
    defined(_M_X64) || \
    defined(_M_IX86)

  #if defined(__POPCNT__)
    #if defined(HAS_POPCNT)
      std::cout << "OK: __POPCNT__ and HAS_POPCNT are defined!" << std::endl;
    #else
      std::cerr << "Error: HAS_POPCNT must be defined if __POPCNT__ is defined!" << std::endl;
      return 1;
    #endif
  #endif

  #if defined(__AVX__)
    #if defined(HAS_POPCNT)
      std::cout << "OK: __AVX__ and HAS_POPCNT are defined!" << std::endl;
    #else
      std::cerr << "Error: HAS_POPCNT must be defined if __AVX__ is defined!" << std::endl;
      return 1;
    #endif
  #endif

  #if defined(__AVX2__)
    #if defined(HAS_POPCNT)
      std::cout << "OK: __AVX2__ and HAS_POPCNT are defined!" << std::endl;
    #else
      std::cerr << "Error: HAS_POPCNT must be defined if __AVX2__ is defined!" << std::endl;
      return 1;
    #endif
  #endif

  #if defined(HAS_POPCNT)
    #if !defined(ENABLE_CPUID_POPCNT)
      std::cout << "OK: HAS_POPCNT is defined but ENABLE_CPUID_POPCNT is not defined!" << std::endl;
    #else
      std::cerr << "Error: ENABLE_CPUID_POPCNT must not be defined if HAS_POPCNT is defined!" << std::endl;
      return 1;
    #endif
  #endif

  #if !defined(HAS_POPCNT)
    #if defined(ENABLE_CPUID_POPCNT)
      std::cout << "OK: HAS_POPCNT is not defined but ENABLE_CPUID_POPCNT is defined!" << std::endl;
    #else
      std::cerr << "Error: ENABLE_CPUID_POPCNT must be defined if HAS_POPCNT is not defined!" << std::endl;
      return 1;
    #endif
  #endif

  #if defined(ENABLE_CPUID_POPCNT)
    std::cout << "CPU supports POPCNT: " << (cpu_supports_popcnt ? "yes" : "no") << std::endl;
  #endif

#endif

  std::cout << std::endl;
  std::cout << "All tests passed successfully!" << std::endl;

  return 0;
}
