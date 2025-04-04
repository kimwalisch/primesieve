# We use GCC/Clang's function multi-versioning for AVX512
# support. This code will automatically dispatch to the
# AVX512 BW algorithm if the CPU supports it and use the
# default (portable) algorithm otherwise.

include(CheckCXXSourceCompiles)
include(CMakePushCheckState)

cmake_push_check_state()
set(CMAKE_REQUIRED_INCLUDES "${PROJECT_SOURCE_DIR}")

check_cxx_source_compiles("
    // GCC/Clang function multiversioning for AVX512 is not needed if
    // the user compiles with -mavx512f -mavx512bw.
    // GCC/Clang function multiversioning generally causes a minor
    // overhead, hence we disable it if it is not needed.
    #if defined(__AVX512F__) && \
        defined(__AVX512BW__)
      Error: AVX512BW multiarch not needed!
    #endif

    #include <src/arch/x86/cpuid.cpp>
    #include <immintrin.h>
    #include <stdint.h>
    #include <cstddef>

    __attribute__ ((target (\"avx512f,avx512bw\")))
    void presieve1_x86_avx512(const uint8_t* __restrict preSieve0,
                              const uint8_t* __restrict preSieve1,
                              uint8_t* __restrict sieve,
                              std::size_t bytes)
    {
        std::size_t i = 0;

        for (; i + 64 <= bytes; i += sizeof(__m512i))
        {
            _mm512_storeu_epi8((__m512i*) &sieve[i],
                _mm512_and_si512(_mm512_loadu_epi8((const __m512i*) &preSieve0[i]), 
                                 _mm512_loadu_epi8((const __m512i*) &preSieve1[i])));
        }

        if (i < bytes)
        {
            __mmask64 mask = 0xffffffffffffffffull >> (i + 64 - bytes);

            _mm512_mask_storeu_epi8((__m512i*) &sieve[i], mask,
                _mm512_and_si512(_mm512_maskz_loadu_epi8(mask, (const __m512i*) &preSieve0[i]), 
                                 _mm512_maskz_loadu_epi8(mask, (const __m512i*) &preSieve1[i])));
        }
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

        if (primesieve::has_cpuid_avx512_bw())
            presieve1_x86_avx512(&PreSieveTable1[0], &PreSieveTable2[1], &sieve[0], 10);
        else
            presieve1_default(&PreSieveTable1[0], &PreSieveTable2[1], &sieve[0], 10);

        return (sieve[0] == 0) ? 0 : 1;
    }
" multiarch_avx512_bw)

if(multiarch_avx512_bw)
    list(APPEND PRIMESIEVE_COMPILE_DEFINITIONS "ENABLE_MULTIARCH_AVX512_BW")
endif()

cmake_pop_check_state()
