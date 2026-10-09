# We use GCC/Clang's function multi-versioning for AVX2
# support. This code will automatically dispatch to the
# AVX2 algorithm if the CPU supports it and use the
# default (portable) algorithm otherwise.

include(CheckCXXSourceCompiles)
include(CMakePushCheckState)

cmake_push_check_state()
set(CMAKE_REQUIRED_INCLUDES "${PROJECT_SOURCE_DIR}")

check_cxx_source_compiles("
    // GCC/Clang function multiversioning for AVX2 is not needed if
    // the user compiles with -mavx2 -mbmi2 -mpopcnt.
    // GCC/Clang function multiversioning generally causes a minor
    // overhead, hence we disable it if it is not needed.
    #if defined(__AVX2__) && \
        defined(__BMI2__) && \
        defined(__POPCNT__)
      Error: AVX2 multiarch not needed!
    #endif

    // AVX2 multiarch is also not needed if the compiler
    // flags already enable the faster AVX512 algorithm.
    #if defined(__AVX512F__) && \
        defined(__AVX512VBMI__) && \
        defined(__AVX512VBMI2__)
      Error: AVX2 multiarch not needed!
    #endif

    #include <src/arch/x86/cpuid.cpp>
    #include <immintrin.h>
    #include <stdint.h>

    class PrimeGenerator {
    public:
        __attribute__ ((target (\"avx2,bmi2,popcnt\")))
        void fillNextPrimes_x86_avx2(uint64_t* primes64);
        void fillNextPrimes_default(uint64_t* primes64);
        void fillNextPrimes(uint64_t* primes64)
        {
            if (primesieve::has_avx2())
                fillNextPrimes_x86_avx2(primes64);
            else
                fillNextPrimes_default(primes64);
        }
    };

    void PrimeGenerator::fillNextPrimes_default(uint64_t* primes64)
    {
        primes64[0] = 2;
    }

    __attribute__ ((target (\"avx2,bmi2,popcnt\")))
    void PrimeGenerator::fillNextPrimes_x86_avx2(uint64_t* primes64)
    {
        alignas(16) uint16_t buffer[16] = { 7, 11, 13, 17, 19, 23, 29, 31 };
        uint64_t bits64 = primes64[0];
        uint64_t pos = _mm_popcnt_u64(_bzhi_u64(bits64, 8));
        __m128i offsets = _mm_set1_epi16(30);
        __m128i bitValues = _mm_load_si128((const __m128i*) buffer);
        _mm_storeu_si128((__m128i*) &buffer[pos], _mm_add_epi16(offsets, bitValues));
        __m256i base = _mm256_set1_epi64x(123);
        __m256i offsets64 = _mm256_cvtepu16_epi64(_mm_loadl_epi64((const __m128i*) buffer));
        _mm256_storeu_si256((__m256i*) primes64, _mm256_add_epi64(base, offsets64));
    }

    int main()
    {
        uint64_t primes[8] = { 0xff };
        PrimeGenerator p;
        p.fillNextPrimes(primes);
        return 0;
    }
" multiarch_avx2)

if(multiarch_avx2)
    list(APPEND PRIMESIEVE_COMPILE_DEFINITIONS "ENABLE_MULTIARCH_AVX2")
endif()

cmake_pop_check_state()
