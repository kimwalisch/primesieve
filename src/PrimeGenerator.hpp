///
/// @file   PrimeGenerator.hpp
/// @brief  Generates the primes inside [start, stop] and stores them
///         in a vector. After the primes have been stored in the
///         vector primesieve::iterator iterates over the vector and
///         returns the primes. When there are no more primes left in
///         the vector PrimeGenerator generates new primes.
///
/// Copyright (C) 2026 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRIMEGENERATOR_HPP
#define PRIMEGENERATOR_HPP

#include "Erat.hpp"
#include "MemoryPool.hpp"
#include "SievingPrimes.hpp"

#include <primesieve/cpu_arch_macros.hpp>
#include <primesieve/macros.hpp>
#include <primesieve/Vector.hpp>

#include <stdint.h>
#include <cstddef>

#if defined(ENABLE_AVX512_VBMI2)
  #define PRIMEGENERATOR_DEFAULT_HEADER "PrimeGenerator_x86_avx512.hpp"
  #define PRIMEGENERATOR_FILL_NEXT_DEFAULT fillNextPrimes_x86_avx512
  #define PRIMEGENERATOR_FILL_PREV_DEFAULT fillPrevPrimes_x86_avx512
#elif defined(ENABLE_AVX2_BMI2)
  #define PRIMEGENERATOR_DEFAULT_HEADER "PrimeGenerator_x86_avx2.hpp"
  #define PRIMEGENERATOR_FILL_NEXT_DEFAULT fillNextPrimes_x86_avx2
  #define PRIMEGENERATOR_FILL_PREV_DEFAULT fillPrevPrimes_x86_avx2
#elif defined(ENABLE_ARM_NEON)
  #define PRIMEGENERATOR_DEFAULT_HEADER "PrimeGenerator_arm_neon.hpp"
  #define PRIMEGENERATOR_FILL_NEXT_DEFAULT fillNextPrimes_arm_neon
  #define PRIMEGENERATOR_FILL_PREV_DEFAULT fillPrevPrimes_arm_neon
#else
  #define PRIMEGENERATOR_DEFAULT_HEADER "PrimeGenerator_default.hpp"
  #define PRIMEGENERATOR_FILL_NEXT_DEFAULT fillNextPrimes_default
  #define PRIMEGENERATOR_FILL_PREV_DEFAULT fillPrevPrimes_default
#endif

#if defined(ENABLE_MULTIARCH_AVX512_VBMI2)
  #include <primesieve/cpu_supports_avx512_vbmi2.hpp>
#endif

#if defined(ENABLE_MULTIARCH_AVX2_BMI2)
  #include <primesieve/cpu_supports_avx2.hpp>
#endif

namespace primesieve {

class PrimeGenerator : public Erat
{
public:
  PrimeGenerator(uint64_t start, uint64_t stop);
  static uint64_t maxCachedPrime();

  ALWAYS_INLINE void fillNextPrimes(Vector<uint64_t>& primes, std::size_t* size)
  {
    #if defined(ENABLE_MULTIARCH_AVX512_VBMI2)
      if (cpu_supports_avx512_vbmi2)
        return fillNextPrimes_x86_avx512(primes, size);
    #endif

    #if defined(ENABLE_MULTIARCH_AVX2_BMI2)
      if (cpu_supports_avx2)
        return fillNextPrimes_x86_avx2(primes, size);
    #endif

    PRIMEGENERATOR_FILL_NEXT_DEFAULT(primes, size);
  }

  ALWAYS_INLINE void fillPrevPrimes(Vector<uint64_t>& primes, std::size_t* size)
  {
    #if defined(ENABLE_MULTIARCH_AVX512_VBMI2)
      if (cpu_supports_avx512_vbmi2)
        return fillPrevPrimes_x86_avx512(primes, size);
    #endif

    #if defined(ENABLE_MULTIARCH_AVX2_BMI2)
      if (cpu_supports_avx2)
        return fillPrevPrimes_x86_avx2(primes, size);
    #endif

    PRIMEGENERATOR_FILL_PREV_DEFAULT(primes, size);
  }

private:
  void PRIMEGENERATOR_FILL_NEXT_DEFAULT(Vector<uint64_t>& primes, std::size_t* size);
  void PRIMEGENERATOR_FILL_PREV_DEFAULT(Vector<uint64_t>& primes, std::size_t* size);

#if defined(ENABLE_MULTIARCH_AVX512_VBMI2)
  __attribute__ ((target ("avx512f,avx512vbmi,avx512vbmi2,popcnt")))
  void fillNextPrimes_x86_avx512(Vector<uint64_t>& primes, std::size_t* size);
  __attribute__ ((target ("avx512f,avx512vbmi,avx512vbmi2,popcnt")))
  void fillPrevPrimes_x86_avx512(Vector<uint64_t>& primes, std::size_t* size);
#endif

#if defined(ENABLE_MULTIARCH_AVX2_BMI2)
  __attribute__ ((target ("avx2,bmi2,popcnt")))
  void fillNextPrimes_x86_avx2(Vector<uint64_t>& primes, std::size_t* size);
  __attribute__ ((target ("avx2,bmi2,popcnt")))
  void fillPrevPrimes_x86_avx2(Vector<uint64_t>& primes, std::size_t* size);
#endif

  bool isInit_ = false;
  uint64_t low_ = 0;
  uint64_t prime_ = 0;
  uint64_t sieveIdx_ = ~0ull;
  MemoryPool memoryPool_;
  SievingPrimes sievingPrimes_;
  std::size_t getStartIdx() const;
  std::size_t getStopIdx() const;
  void initErat();
  void sieveSegment();
  void initPrevPrimes(Vector<uint64_t>&, std::size_t*);
  void initNextPrimes(Vector<uint64_t>&, std::size_t*);
  bool sievePrevPrimes(Vector<uint64_t>&, std::size_t*);
  bool sieveNextPrimes(Vector<uint64_t>&, std::size_t*);
};

} // namespace

#endif
