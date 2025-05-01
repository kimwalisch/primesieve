///
/// @file   IteratorHelper.hpp
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef ITERATOR_HELPER_HPP
#define ITERATOR_HELPER_HPP

#include "PrimeGenerator.hpp"
#include <primesieve/macros.hpp>
#include <primesieve/Vector.hpp>

#include <stdint.h>

namespace primesieve {

// These objects can be reused by primesieve::iterator
// and don't need to be reallocated frequently.
struct IteratorData
{
  IteratorData(uint64_t stp) :
    stop(stp)
  { }

  ~IteratorData()
  {
    if (primeGenerator)
      primeGenerator->~PrimeGenerator();
  }

  void deletePrimeGenerator()
  {
    if (primeGenerator)
    {
      primeGenerator->~PrimeGenerator();
      primeGenerator = nullptr;
    }
  }

  void deletePrimes()
  {
    primes.deallocate();
  }

  void newPrimeGenerator(uint64_t start,
                         uint64_t stop)
  {
    // We use placement new to put the PrimeGenerator
    // into an existing buffer. This way we don't
    // need to allocate any new memory.
    ASSERT(primeGenerator == nullptr);
    primeGenerator = new (primeGeneratorBuffer) PrimeGenerator(start, stop);
  }

  uint64_t stop;
  uint64_t dist = 0;
  bool include_start_number = true;
  PrimeGenerator* primeGenerator = nullptr;
  Vector<uint64_t> primes;
  alignas(PrimeGenerator) char primeGeneratorBuffer[sizeof(PrimeGenerator)];
};

class IteratorHelper
{
public:
  static void updateNext(uint64_t& start,
                         uint64_t stopHint,
                         IteratorData& iter);

  static void updatePrev(uint64_t& start,
                         uint64_t stopHint,
                         IteratorData& iter);
};

} // namespace

#endif
