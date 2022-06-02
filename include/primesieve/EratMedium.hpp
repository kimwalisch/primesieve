///
/// @file  EratMedium.hpp
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef ERATMEDIUM_HPP
#define ERATMEDIUM_HPP

#include "Bucket.hpp"
#include "macros.hpp"
#include "pod_vector.hpp"
#include "Wheel.hpp"

#include <stdint.h>

namespace primesieve {

class MemoryPool;

/// EratMedium is an implementation of the segmented sieve of
/// Eratosthenes optimized for medium sieving primes
/// that have a few multiples per segment.
///
class EratMedium : public Wheel30_t
{
public:
  void init(uint64_t, uint64_t, MemoryPool&);
  bool hasSievingPrimes() const { return hasSievingPrimes_; }
  NOINLINE void crossOff(pod_vector<uint8_t>& sieve);
private:
  bool hasSievingPrimes_ = false;
  uint64_t maxPrime_ = 0;
  MemoryPool* memoryPool_ = nullptr;
  pod_array<SievingPrime*, 64> buckets_;
  void storeSievingPrime(uint64_t, uint64_t, uint64_t);
  NOINLINE void crossOff_7(uint8_t*, uint8_t*, Bucket*);
  NOINLINE void crossOff_11(uint8_t*, uint8_t*, Bucket*);
  NOINLINE void crossOff_13(uint8_t*, uint8_t*, Bucket*);
  NOINLINE void crossOff_17(uint8_t*, uint8_t*, Bucket*);
  NOINLINE void crossOff_19(uint8_t*, uint8_t*, Bucket*);
  NOINLINE void crossOff_23(uint8_t*, uint8_t*, Bucket*);
  NOINLINE void crossOff_29(uint8_t*, uint8_t*, Bucket*);
  NOINLINE void crossOff_31(uint8_t*, uint8_t*, Bucket*);
};

} // namespace

#endif
