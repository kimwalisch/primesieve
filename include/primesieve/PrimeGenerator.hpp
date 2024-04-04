///
/// @file   PrimeGenerator.hpp
/// @brief  Generates the primes inside [start, stop] and stores them
///         in a vector. After the primes have been stored in the
///         vector primesieve::iterator iterates over the vector and
///         returns the primes. When there are no more primes left in
///         the vector PrimeGenerator generates new primes.
///
/// Copyright (C) 2024 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRIMEGENERATOR_HPP
#define PRIMEGENERATOR_HPP

#include "Erat.hpp"
#include "MemoryPool.hpp"
#include "SievingPrimes.hpp"
#include "Vector.hpp"

#include <stdint.h>
#include <cstddef>

#if defined(MULTIARCH_AVX512)
  #if defined(__AVX512F__) && \
      defined(__AVX512VBMI__) && \
      defined(__AVX512VBMI2__)
    // GCC/Clang function multiversioning for AVX512 is not needed if
    // the user compiles with -mavx512f -mavx512vbmi -mavx512vbmi2.
    // GCC/Clang function multiversioning generally causes a minor
    // overhead, hence we disable it if it is not needed.
    #undef MULTIARCH_AVX512
  #else
    #define MULTIARCH_TARGET_DEFAULT
    #define MULTIARCH_TARGET_AVX512
  #endif
#endif

namespace primesieve {

class PreSieve;

class PrimeGenerator : public Erat
{
public:
  PrimeGenerator(uint64_t start, uint64_t stop, PreSieve& preSieve);
  void fillPrevPrimes(Vector<uint64_t>& primes, std::size_t* size);
  static uint64_t maxCachedPrime();

#if defined(MULTIARCH_TARGET_DEFAULT)
  __attribute__ ((target ("default")))
#endif
  void fillNextPrimes(Vector<uint64_t>& primes, std::size_t* size);

#if defined(MULTIARCH_TARGET_AVX512)
  __attribute__ ((target ("avx512f,avx512vbmi,avx512vbmi2")))
  void fillNextPrimes(Vector<uint64_t>& primes, std::size_t* size);
#endif

private:
  bool isInit_ = false;
  uint64_t low_ = 0;
  uint64_t prime_ = 0;
  uint64_t sieveIdx_ = ~0ull;
  PreSieve& preSieve_;
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
