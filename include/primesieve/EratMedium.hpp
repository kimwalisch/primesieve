///
/// @file  EratMedium.hpp
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef ERATMEDIUM_HPP
#define ERATMEDIUM_HPP

#include "Bucket.hpp"
#include "MemoryPool.hpp"
#include "types.hpp"
#include "Wheel.hpp"

#include <stdint.h>
#include <array>

namespace primesieve {

/// EratMedium is an implementation of the segmented sieve of
/// Eratosthenes optimized for medium sieving primes
/// that have a few multiples per segment.
///
class EratMedium : public Wheel30_t
{
public:
  void init(uint64_t, uint64_t, uint64_t);
  bool enabled() const { return enabled_; }
  void crossOff(byte_t*, uint64_t);
private:
  bool enabled_ = false;
  uint64_t maxPrime_;
  MemoryPool memoryPool_;
  std::array<SievingPrime*, 64> sievingPrimes_;
  void resetSievingPrimes();
  void storeSievingPrime(uint64_t, uint64_t, uint64_t);
  void crossOff(byte_t*, byte_t*, Bucket*);
  void crossOff_7(byte_t*, byte_t*, Bucket*);
  void crossOff_11(byte_t*, byte_t*, Bucket*);
  void crossOff_13(byte_t*, byte_t*, Bucket*);
  void crossOff_17(byte_t*, byte_t*, Bucket*);
  void crossOff_19(byte_t*, byte_t*, Bucket*);
  void crossOff_23(byte_t*, byte_t*, Bucket*);
  void crossOff_29(byte_t*, byte_t*, Bucket*);
  void crossOff_31(byte_t*, byte_t*, Bucket*);
};

} // namespace

#endif
