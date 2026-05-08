# We use GCC/Clang's function multi-versioning for AVX2
# support. This code will automatically dispatch to the
# AVX2 algorithm if the CPU supports it and use the
# default (portable) algorithm otherwise.
# AVX2 is available on most modern x86 CPUs (~95%):
# - Intel Haswell (2013+) and later
# - AMD Zen (2017+) and later

include(CheckCXXSourceCompiles)
include(CMakePushCheckState)

cmake_push_check_state()
set(CMAKE_REQUIRED_INCLUDES "${PROJECT_SOURCE_DIR}")

check_cxx_source_compiles("
    // GCC/Clang function multiversioning for AVX2 is not needed if
    // the user compiles with -mavx2.
    // GCC/Clang function multiversioning generally causes a minor
    // overhead, hence we disable it if it is not needed.
    #if defined(__AVX2__)
      Error: AVX2 multiarch not needed!
    #endif

    #include <src/arch/x86/cpuid.cpp>
    #include <immintrin.h>
    #include <stdint.h>
    #include <cstddef>

    void presieve1_x86_avx2(const uint8_t* __restrict preSieve0,
                            const uint8_t* __restrict preSieve1,
                            uint8_t* __restrict sieve,
                            std::size_t bytes)
    {
        std::size_t i = 0;

        for (; i + 32 <= bytes; i += sizeof(__m256i))
        {
            _mm256_storeu_si256((__m256i*) &sieve[i],
                _mm256_and_si256(_mm256_loadu_si256((const __m256i*) &preSieve0[i]),
                                 _mm256_loadu_si256((const __m256i*) &preSieve1[i])));
        }

        for (; i < bytes; i++)
            sieve[i] = preSieve0[i] & preSieve1[i];
    }

    void presieve1_default(const uint8_t* __restrict preSieved0,
                           const uint8_t* __restrict preSieved1,
                           uint8_t* __restrict sieve,
                           std::size_t bytes)
    {
        for (std::size_t i = 0; i < bytes; i++)
            sieve[i] = preSieved0[i] & preSieved1[i];
    }

    int main()
    {
        uint8_t sieve[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        uint8_t PreSieveTable1[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        uint8_t PreSieveTable2[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

        if (primesieve::has_cpuid_avx2())
            presieve1_x86_avx2(&PreSieveTable1[0], &PreSieveTable2[1], &sieve[0], 10);
        else
            presieve1_default(&PreSieveTable1[0], &PreSieveTable2[1], &sieve[0], 10);

        return (sieve[0] == 0) ? 0 : 1;
    }
" multiarch_avx2)

if(multiarch_avx2)
    list(APPEND PRIMESIEVE_COMPILE_DEFINITIONS "ENABLE_MULTIARCH_AVX2")
endif()

cmake_pop_check_state()