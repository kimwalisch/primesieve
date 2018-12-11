///
/// @file  EratBig.hpp
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef ERATBIG_HPP
#define ERATBIG_HPP

#include "Bucket.hpp"
#include "MemoryPool.hpp"
#include "Wheel.hpp"
#include "types.hpp"

#include <stdint.h>
#include <vector>

namespace primesieve {

/// EratBig is an implementation of the segmented sieve of
/// Eratosthenes optimized for big sieving primes that have
/// very few multiples per segment.
///
class EratBig : public Wheel210_t
{
public:
  void init(uint64_t, uint64_t, uint64_t);
  void crossOff(byte_t*);
  bool enabled() const { return enabled_; }
private:
  uint64_t maxPrime_;
  uint64_t log2SieveSize_;
  uint64_t moduloSieveSize_;
  std::vector<SievingPrime*> sievingPrimes_;
  MemoryPool memoryPool_;
  bool enabled_ = false;
  void init(uint64_t);
  void storeSievingPrime(uint64_t, uint64_t, uint64_t);
  void crossOff(byte_t*, Bucket*);
};

} // namespace

#endif
