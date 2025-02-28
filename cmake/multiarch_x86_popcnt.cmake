# On x86 CPUs we need to enable the use of cpuid.cpp.
# If cpuid.cpp compiles we assume it is a x86 CPU.

include(CheckCXXSourceCompiles)
include(CMakePushCheckState)

cmake_push_check_state()
set(CMAKE_REQUIRED_INCLUDES "${PROJECT_SOURCE_DIR}")

check_cxx_source_compiles("
    // Enable CPUID for POPCNT on x86 and x86-64 CPUs.
    // This is required because not all x86 and x86-64 CPUs
    // support the POPCNT instruction.
    #if !(defined(__x86_64__) || \
          defined(__i386__) || \
          defined(_M_X64) || \
          defined(_M_IX86))
      Error: x86 POPCNT multiarch not needed!
    #endif

    // Both GCC and Clang (even Clang on Windows) define the __POPCNT__
    // macro if the user compiles with -mpopcnt. The __POPCNT__
    // macro is even defined if the user compiles with other flags
    // such as -mavx or -march=native.
    #if defined(__POPCNT__)
        Error: x86 POPCNT multiarch not needed!

    // The MSVC compiler does not support a POPCNT macro, but if the user
    // compiles with e.g. /arch:AVX or /arch:AVX512 then MSVC defines
    // the __AVX__ macro and POPCNT is also supported.
    #elif defined(_MSC_VER) && defined(__AVX__)
        Error: x86 POPCNT multiarch not needed!
    #endif

    #include <src/arch/x86/cpuid.cpp>
    #include <iostream>

    int main()
    {
        if (primesieve::has_cpuid_popcnt())
            std::cout << \"CPU supports POPCNT!\" << std::endl;
        else
            std::cout << \"CPU does not support POPCNT!\" << std::endl;

        return 0;
    }
" multiarch_x86_popcnt)

if(multiarch_x86_popcnt)
    list(APPEND PRIMESIEVE_COMPILE_DEFINITIONS "ENABLE_MULTIARCH_x86_POPCNT")
endif()

cmake_pop_check_state()
