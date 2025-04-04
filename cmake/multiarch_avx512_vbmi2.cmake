# We use GCC/Clang's function multi-versioning for AVX512
# support. This code will automatically dispatch to the
# AVX512 VBMI2 algorithm if the CPU supports it and use
# the default (portable) algorithm otherwise.

include(CheckCXXSourceCompiles)
include(CMakePushCheckState)

cmake_push_check_state()
set(CMAKE_REQUIRED_INCLUDES "${PROJECT_SOURCE_DIR}")

check_cxx_source_compiles("
    // GCC/Clang function multiversioning for AVX512 is not needed if
    // the user compiles with -mavx512f -mavx512vbmi -mavx512vbmi2.
    // GCC/Clang function multiversioning generally causes a minor
    // overhead, hence we disable it if it is not needed.
    #if defined(__AVX512F__) && \
        defined(__AVX512VBMI__) && \
        defined(__AVX512VBMI2__)
      Error: AVX512VBMI2 multiarch not needed!
    #endif

    #include <src/arch/x86/cpuid.cpp>
    #include <immintrin.h>
    #include <stdint.h>

    class PrimeGenerator {
    public:
        __attribute__ ((target (\"avx512f,avx512vbmi,avx512vbmi2\")))
        void fillNextPrimes_x86_avx512(uint64_t* primes64);
        void fillNextPrimes_default(uint64_t* primes64);
        void fillNextPrimes(uint64_t* primes64)
        {
            if (primesieve::has_cpuid_avx512_vbmi2())
                fillNextPrimes_x86_avx512(primes64);
            else
                fillNextPrimes_default(primes64);
        }
    };

    void PrimeGenerator::fillNextPrimes_default(uint64_t* primes64)
    {
        primes64[0] = 2;
    }

    __attribute__ ((target (\"avx512f,avx512vbmi,avx512vbmi2\")))
    void PrimeGenerator::fillNextPrimes_x86_avx512(uint64_t* primes64)
    {
        __m512i bytes_0_to_7 = _mm512_setr_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        __m512i base = _mm512_set1_epi64(123);
        __m512i bitValues = _mm512_maskz_compress_epi8(0xffff, base);
        __m512i vprimes = _mm512_maskz_permutexvar_epi8(0x0101010101010101ull, bytes_0_to_7, bitValues);
        vprimes = _mm512_add_epi64(base, vprimes);
        _mm512_storeu_si512(primes64, vprimes);
    }

    int main()
    {
        uint64_t primes[8];
        PrimeGenerator p;
        p.fillNextPrimes(primes);
        return 0;
    }
" multiarch_avx512_vbmi2)

if(multiarch_avx512_vbmi2)
    list(APPEND PRIMESIEVE_COMPILE_DEFINITIONS "ENABLE_MULTIARCH_AVX512_VBMI2")
endif()

cmake_pop_check_state()
