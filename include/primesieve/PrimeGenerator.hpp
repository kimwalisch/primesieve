///
/// @file  PrimeGenerator.hpp
///        Generates the primes inside [start, stop] and stores them
///        in a vector. After the primes have been stored in the
///        vector primesieve::iterator iterates over the vector and
///        returns the primes. When there are no more primes left in
///        the vector PrimeGenerator generates new primes.
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRIMEGENERATOR_HPP
#define PRIMEGENERATOR_HPP

#include "Erat.hpp"
#include "CpuInfo.hpp"
#include "MemoryPool.hpp"
#include "PreSieve.hpp"
#include "SievingPrimes.hpp"

#include <stdint.h>
#include <vector>

// On x64 CPUs we compile with AVX512 support if the
// compiler supports it. At runtime we then dispatch
// to fillNextPrimesAVX512() if the user's CPU supports
// AVX512 or to fillNextPrimesCTZ() otherwise.
#if !defined(DISABLE_AVX512) && \
    (defined(__i386__) || \
     defined(__x86_64__) || \
     defined(_M_IX86) || \
     defined(_M_X64)) && \
     __has_include(<immintrin.h>)

  #if defined(_MSC_VER)
    // AVX512VBMI2 requires MSVC 2019 or later
    #if _MSC_VER >= 1920
      #define ENABLE_AVX512
    #endif
  #elif defined(__clang__)
    #define CLANG_PREREQ(x, y) \
      (__clang_major__ > x || (__clang_major__ == x && __clang_minor__ >= y))
    // AVX512VBMI2 requires Clang 6.x or later.
    // Apple Clang used to report erroneous versions,
    // therefore we require Clang 11.0 (though it
    // seems like 9.1 would be enough).
    // See: https://gist.github.com/yamaya/2924292
    #if CLANG_PREREQ(11, 0)
      #define ENABLE_AVX512
    #endif
  #elif defined(__GNUC__)
    #define GNUC_PREREQ(x, y) \
      (__GNUC__ > x || (__GNUC__ == x && __GNUC_MINOR__ >= y))
    // AVX512VBMI2 requires GCC 8.x or later
    #if GNUC_PREREQ(8, 0)
      #define ENABLE_AVX512
    #endif
  #endif
#endif

namespace primesieve {

class PrimeGenerator : public Erat
{
public:
  PrimeGenerator(uint64_t start, uint64_t stop);
  void fillPrevPrimes(std::vector<uint64_t>& primes, std::size_t* size);
  static uint64_t maxCachedPrime();

  void fillNextPrimes(std::vector<uint64_t>& primes, std::size_t* size)
  {
    #if defined(ENABLE_AVX512)
      if (cpuInfo.hasAVX512())
        fillNextPrimesAVX512(primes, size);
      else
        fillNextPrimesCTZ(primes, size);
    #else
        fillNextPrimesCTZ(primes, size);
    #endif
  }

private:
  uint64_t low_ = 0;
  uint64_t sieveIdx_ = ~0ull;
  uint64_t prime_ = 0;
  MemoryPool memoryPool_;
  PreSieve preSieve_;
  SievingPrimes sievingPrimes_;
  bool isInit_ = false;
  std::size_t getStartIdx() const;
  std::size_t getStopIdx() const;
  void initErat();
  void sieveSegment();
  void initPrevPrimes(std::vector<uint64_t>&, std::size_t*);
  void initNextPrimes(std::vector<uint64_t>&, std::size_t*);
  bool sievePrevPrimes(std::vector<uint64_t>&, std::size_t*);
  bool sieveNextPrimes(std::vector<uint64_t>&, std::size_t*);
  void fillNextPrimesCTZ(std::vector<uint64_t>& primes, std::size_t* size);

#if defined(ENABLE_AVX512)
  void fillNextPrimesAVX512(std::vector<uint64_t>& primes, std::size_t* size);
#endif
};

} // namespace

#endif
