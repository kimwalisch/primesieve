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
#include "pod_vector.hpp"
#include "Wheel.hpp"

#include <stdint.h>

namespace primesieve {

class MemoryPool;

/// EratBig is an implementation of the segmented sieve of
/// Eratosthenes optimized for big sieving primes that have
/// very few multiples per segment.
///
class EratBig : public Wheel210_t
{
public:
  void init(uint64_t, uint64_t, uint64_t, MemoryPool&);
  NOINLINE void crossOff(pod_vector<uint8_t>& sieve);
  bool hasSievingPrimes() const { return !buckets_.empty(); }
private:
  uint64_t maxPrime_ = 0;
  uint64_t log2SieveSize_ = 0;
  uint64_t moduloSieveSize_ = 0;
  MemoryPool* memoryPool_ = nullptr;
  pod_vector<SievingPrime*> buckets_;
  void storeSievingPrime(uint64_t, uint64_t, uint64_t);
  NOINLINE void crossOff(uint8_t* sieve, SievingPrime* prime, SievingPrime* end);
};

} // namespace

#endif
