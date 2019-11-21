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
  void crossOff(uint8_t*, uint64_t);
private:
  bool enabled_ = false;
  uint64_t maxPrime_ = 0;
  MemoryPool memoryPool_;
  std::array<SievingPrime*, 64> sievingPrimes_;
  void resetSievingPrimes();
  void storeSievingPrime(uint64_t, uint64_t, uint64_t);
  void crossOff(uint8_t*, uint8_t*, Bucket*);
  void crossOff_7(uint8_t*, uint8_t*, Bucket*);
  void crossOff_11(uint8_t*, uint8_t*, Bucket*);
  void crossOff_13(uint8_t*, uint8_t*, Bucket*);
  void crossOff_17(uint8_t*, uint8_t*, Bucket*);
  void crossOff_19(uint8_t*, uint8_t*, Bucket*);
  void crossOff_23(uint8_t*, uint8_t*, Bucket*);
  void crossOff_29(uint8_t*, uint8_t*, Bucket*);
  void crossOff_31(uint8_t*, uint8_t*, Bucket*);
};

} // namespace

#endif
