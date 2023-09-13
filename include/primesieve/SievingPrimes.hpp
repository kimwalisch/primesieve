///
/// @file  SievingPrimes.hpp
///
/// Copyright (C) 2023 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef SIEVINGPRIMES_HPP
#define SIEVINGPRIMES_HPP

#include "Erat.hpp"
#include "macros.hpp"
#include "Vector.hpp"

#include <stdint.h>

namespace primesieve {

class PreSieve;
class MemoryPool;

class SievingPrimes : public Erat
{
public:
  SievingPrimes() = default;
  SievingPrimes(Erat*, uint64_t, PreSieve&, MemoryPool& memoryPool);
  void init(Erat*, uint64_t, PreSieve&, MemoryPool& memoryPool);
  uint64_t next();
private:
  uint64_t i_ = 0;
  uint64_t size_ = 0;
  uint64_t low_ = 0;
  uint64_t tinyIdx_ = 0;
  uint64_t sieveIdx_ = ~0ull;
  Array<uint64_t, 128> primes_;
  Vector<bool> tinySieve_;
  NOINLINE void fill();
  void tinySieve();
  bool sieveSegment();
};

inline uint64_t SievingPrimes::next()
{
  while (i_ >= size_)
    fill();

  return primes_[i_++];
}

} // namespace

#endif
