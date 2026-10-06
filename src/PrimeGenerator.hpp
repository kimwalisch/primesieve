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

#include <primesieve/macros.hpp>
#include <primesieve/Vector.hpp>

#include <stdint.h>
#include <cstddef>

#if defined(__AVX512F__) && \
    defined(__AVX512VBMI__) && \
    defined(__AVX512VBMI2__) && \
    __has_include(<immintrin.h>)
  #define ENABLE_AVX512_VBMI2
#elif defined(ENABLE_MULTIARCH_AVX512_VBMI2)
  #include <primesieve/cpu_supports_avx512_vbmi2.hpp>
#endif

#if defined(__aarch64__) && \
    __has_include(<arm_neon.h>)
  #define PRIMEGENERATOR_DEFAULT_HEADER "PrimeGenerator_arm_neon.hpp"
  #define PRIMEGENERATOR_FILL_NEXT_DEFAULT fillNextPrimes_arm_neon
  #define PRIMEGENERATOR_FILL_PREV_DEFAULT fillPrevPrimes_arm_neon
#else
  #define PRIMEGENERATOR_DEFAULT_HEADER "PrimeGenerator_default.hpp"
  #define PRIMEGENERATOR_FILL_NEXT_DEFAULT fillNextPrimes_default
  #define PRIMEGENERATOR_FILL_PREV_DEFAULT fillPrevPrimes_default
#endif

namespace primesieve {

class PrimeGenerator : public Erat
{
public:
  PrimeGenerator(uint64_t start, uint64_t stop);
  static uint64_t maxCachedPrime();

  ALWAYS_INLINE void fillNextPrimes(Vector<uint64_t>& primes, std::size_t* size)
  {
    #if defined(ENABLE_AVX512_VBMI2)
      fillNextPrimes_x86_avx512(primes, size);

    #elif defined(ENABLE_MULTIARCH_AVX512_VBMI2)
      if (cpu_supports_avx512_vbmi2)
        fillNextPrimes_x86_avx512(primes, size);
      else
        PRIMEGENERATOR_FILL_NEXT_DEFAULT(primes, size);

    #else
      PRIMEGENERATOR_FILL_NEXT_DEFAULT(primes, size);
    #endif
  }

  ALWAYS_INLINE void fillPrevPrimes(Vector<uint64_t>& primes, std::size_t* size)
  {
    #if defined(ENABLE_AVX512_VBMI2)
      fillPrevPrimes_x86_avx512(primes, size);

    #elif defined(ENABLE_MULTIARCH_AVX512_VBMI2)
      if (cpu_supports_avx512_vbmi2)
        fillPrevPrimes_x86_avx512(primes, size);
      else
        PRIMEGENERATOR_FILL_PREV_DEFAULT(primes, size);

    #else
      PRIMEGENERATOR_FILL_PREV_DEFAULT(primes, size);
    #endif
  }

private:

#if !defined(ENABLE_AVX512_VBMI2)
  void PRIMEGENERATOR_FILL_NEXT_DEFAULT(Vector<uint64_t>& primes, std::size_t* size);
  void PRIMEGENERATOR_FILL_PREV_DEFAULT(Vector<uint64_t>& primes, std::size_t* size);
#endif

#if defined(ENABLE_AVX512_VBMI2) || \
    defined(ENABLE_MULTIARCH_AVX512_VBMI2)

  #if defined(ENABLE_MULTIARCH_AVX512_VBMI2)
    __attribute__ ((target ("avx512f,avx512vbmi,avx512vbmi2")))
  #endif
  void fillNextPrimes_x86_avx512(Vector<uint64_t>& primes, std::size_t* size);

  #if defined(ENABLE_MULTIARCH_AVX512_VBMI2)
    __attribute__ ((target ("avx512f,avx512vbmi,avx512vbmi2")))
  #endif
  void fillPrevPrimes_x86_avx512(Vector<uint64_t>& primes, std::size_t* size);

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
