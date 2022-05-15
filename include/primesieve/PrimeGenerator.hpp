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
#include "SievingPrimes.hpp"
#include "pod_vector.hpp"

#include <stdint.h>
#include <cstddef>

namespace primesieve {

class PreSieve;

class PrimeGenerator : public Erat
{
public:
  PrimeGenerator(uint64_t start, uint64_t stop, PreSieve& preSieve);
  void fillPrevPrimes(pod_vector<uint64_t>& primes, std::size_t* size);
  static uint64_t maxCachedPrime();

#if defined(ENABLE_AVX512)
  #define FILLNEXTPRIMES_FUNCTION_MULTIVERSIONING
  __attribute__ ((target ("default")))
  void fillNextPrimes(pod_vector<uint64_t>& primes, std::size_t* size);
  __attribute__ ((target ("avx512f,avx512vbmi,avx512vbmi2,popcnt")))
  void fillNextPrimes(pod_vector<uint64_t>& primes, std::size_t* size);
#else
  void fillNextPrimes(pod_vector<uint64_t>& primes, std::size_t* size);
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
  void initPrevPrimes(pod_vector<uint64_t>&, std::size_t*);
  void initNextPrimes(pod_vector<uint64_t>&, std::size_t*);
  bool sievePrevPrimes(pod_vector<uint64_t>&, std::size_t*);
  bool sieveNextPrimes(pod_vector<uint64_t>&, std::size_t*);
};

} // namespace

#endif
