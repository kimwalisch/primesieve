include(CheckCXXSourceCompiles)
include(CMakePushCheckState)

cmake_push_check_state()
set(CMAKE_REQUIRED_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/include")

check_cxx_source_compiles("
    // We need to ensure this code snippet fails on non x64 CPUs
    // (and not simply print a compiler warning).
    #if !defined(__x86_64__)
        Error: not x86-64
    #endif
    #if defined(__POPCNT__) && defined(__BMI__)
        Error: multiarch_popcnt_bmi not needed
    #endif
    #include <primesieve/intrinsics.hpp>
    #include <stdint.h>
    class PrimeGenerator {
        public:
        __attribute__ ((target (\"default\")))
        void fillNextPrimes(uint64_t* primes64);
        __attribute__ ((target (\"popcnt,bmi\")))
        void fillNextPrimes(uint64_t* primes64);
    };
    __attribute__ ((target (\"default\")))
    void PrimeGenerator::fillNextPrimes(uint64_t* primes64)
    {
        primes64[0] = 2;
    }
    __attribute__ ((target (\"popcnt,bmi\")))
    void PrimeGenerator::fillNextPrimes(uint64_t* primes64)
    {
        uint64_t bits = 789;
        uint64_t count = popcnt64(bits);
        if (count > 0) primes64[0] = ctz64(bits); bits &= bits - 1;
        if (count > 1) primes64[1] = ctz64(bits); bits &= bits - 1;
        if (count > 2) primes64[2] = ctz64(bits); bits &= bits - 1;
        if (count > 3) primes64[3] = ctz64(bits); bits &= bits - 1;
    }
    int main()
    {
        uint64_t primes[8];
        PrimeGenerator p;
        p.fillNextPrimes(primes);
        return 0;
    }
" multiarch_popcnt_bmi)

cmake_pop_check_state()
