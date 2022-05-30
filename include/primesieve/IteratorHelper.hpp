///
/// @file   IteratorHelper.hpp
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef ITERATOR_HELPER_HPP
#define ITERATOR_HELPER_HPP

#include "PrimeGenerator.hpp"
#include "PreSieve.hpp"
#include "pod_vector.hpp"

#include <stdint.h>

namespace primesieve {

// These objects can be reused by primesieve::iterator
// and don't need to be reallocated frequently.
struct IteratorMemory
{
  IteratorMemory(uint64_t start) :
    stop(start)
  { }
  ~IteratorMemory()
  {
    delete primeGenerator;
  }
  void deletePrimeGenerator()
  {
    delete primeGenerator;
    primeGenerator = nullptr;
  }
  void deletePrimes()
  {
    primes.free();
  }
  uint64_t stop;
  uint64_t dist = 0;
  PrimeGenerator* primeGenerator = nullptr;
  pod_vector<uint64_t> primes;
  PreSieve preSieve;
};

class IteratorHelper
{
public:
  static void next(uint64_t* start,
                   uint64_t* stop,
                   uint64_t stopHint,
                   uint64_t* dist);

  static void prev(uint64_t* start,
                   uint64_t* stop,
                   uint64_t stopHint,
                   uint64_t* dist);
};

} // namespace

#endif
