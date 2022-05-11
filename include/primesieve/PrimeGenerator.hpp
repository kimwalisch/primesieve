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
#include "MemoryPool.hpp"
#include "PreSieve.hpp"
#include "SievingPrimes.hpp"

#include <stdint.h>
#include <vector>

namespace primesieve {

class PrimeGenerator : public Erat
{
public:
  PrimeGenerator(uint64_t start, uint64_t stop);
  void fillPrevPrimes(std::vector<uint64_t>& primes, std::size_t* size);
  static uint64_t maxCachedPrime();

#if defined(ENABLE_AVX512)
  #define FILLNEXTPRIMES_FUNCTION_MULTIVERSIONING
  __attribute__ ((target ("default")))
  void fillNextPrimes(std::vector<uint64_t>& primes, std::size_t* size);
  __attribute__ ((target ("avx512f,avx512vbmi,avx512vbmi2,popcnt")))
  void fillNextPrimes(std::vector<uint64_t>& primes, std::size_t* size);
#else
  void fillNextPrimes(std::vector<uint64_t>& primes, std::size_t* size);
#endif

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
};

} // namespace

#endif
