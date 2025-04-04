# We use GCC/Clang's function multi-versioning for ARM SVE
# support. This code will automatically dispatch to the
# ARM SVE algorithm if the CPU supports it and use the default
# (portable) algorithm otherwise.

include(CheckCXXSourceCompiles)
include(CMakePushCheckState)

cmake_push_check_state()
set(CMAKE_REQUIRED_INCLUDES "${PROJECT_SOURCE_DIR}")

check_cxx_source_compiles("
    // GCC/Clang function multiversioning for ARM SVE is not needed
    // if the user compiles with -march=armv8-a+sve. GCC/Clang
    // function multiversioning generally causes a minor overhead,
    // hence we disable it if it is not needed.
    #if defined(__ARM_FEATURE_SVE) && \
        __has_include(<arm_sve.h>)
      Error: ARM SVE multiarch not needed!
    #endif

    #include <src/arch/arm/sve.cpp>
    #include <arm_sve.h>
    #include <stdint.h>
    #include <cstddef>

    __attribute__ ((target (\"arch=armv8-a+sve\")))
    void presieve1_arm_sve(const uint8_t* __restrict preSieved0,
                           const uint8_t* __restrict preSieved1,
                           const uint8_t* __restrict preSieved2,
                           const uint8_t* __restrict preSieved3,
                           uint8_t* __restrict sieve,
                           std::size_t bytes)
    {
        for (std::size_t i = 0; i < bytes; i += svcntb())
        {
            svbool_t pg = svwhilelt_b8(i, bytes);

            svst1_u8(pg, &sieve[i],
                svand_u8_x(svptrue_b64(),
                    svand_u8_z(pg, svld1_u8(pg, &preSieved0[i]), svld1_u8(pg, &preSieved1[i])),
                    svand_u8_z(pg, svld1_u8(pg, &preSieved2[i]), svld1_u8(pg, &preSieved3[i]))));
        }
    }

    void presieve1_default(const uint8_t* __restrict preSieved0,
                           const uint8_t* __restrict preSieved1,
                           const uint8_t* __restrict preSieved2,
                           const uint8_t* __restrict preSieved3,
                           uint8_t* __restrict sieve,
                           std::size_t bytes)
    {
        for (std::size_t i = 0; i < bytes; i++)
            sieve[i] = preSieved0[i] & preSieved1[i] & preSieved2[i] & preSieved3[i];
    }

    int main()
    {
        uint8_t sieve[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        uint8_t PreSieveTable1[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        uint8_t PreSieveTable2[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        uint8_t PreSieveTable3[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        uint8_t PreSieveTable4[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

        if (primesieve::has_arm_sve())
            presieve1_arm_sve(&PreSieveTable1[0], &PreSieveTable2[1], &PreSieveTable3[1], &PreSieveTable4[1], &sieve[0], 10);
        else
            presieve1_default(&PreSieveTable1[0], &PreSieveTable2[1], &PreSieveTable3[1], &PreSieveTable4[1], &sieve[0], 10);

        return (sieve[0] == 0) ? 0 : 1;
    }
" multiarch_sve_arm)

if(multiarch_sve_arm)
    list(APPEND PRIMESIEVE_COMPILE_DEFINITIONS "ENABLE_MULTIARCH_ARM_SVE")
endif()

cmake_pop_check_state()
