///
/// @file  EratBig.hpp
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef ERATBIG_HPP
#define ERATBIG_HPP

#include "Bucket.hpp"
#include "macros.hpp"
#include "MemoryPool.hpp"
#include "Wheel.hpp"

#include <cstdint>
#include <vector>

namespace primesieve {

/// EratBig is an implementation of the segmented sieve of
/// Eratosthenes optimized for big sieving primes that have
/// very few multiples per segment.
///
class EratBig : public Wheel210_t
{
public:
  void init(uint64_t, uint64_t, uint64_t, MemoryPool&);
  NOINLINE void crossOff(uint8_t*);
  bool enabled() const { return enabled_; }
private:
  uint64_t maxPrime_ = 0;
  uint64_t log2SieveSize_ = 0;
  uint64_t moduloSieveSize_ = 0;
  MemoryPool* memoryPool_ = nullptr;
  std::vector<SievingPrime*> buckets_;
  bool enabled_ = false;
  void storeSievingPrime(uint64_t, uint64_t, uint64_t);
  NOINLINE void crossOff(uint8_t*, Bucket*);
};

} // namespace

#endif
