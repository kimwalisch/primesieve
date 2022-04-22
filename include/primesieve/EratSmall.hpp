///
/// @file  EratSmall.hpp
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef ERATSMALL_HPP
#define ERATSMALL_HPP

#include "Bucket.hpp"
#include "macros.hpp"
#include "Wheel.hpp"

#include <stdint.h>
#include <vector>

namespace primesieve {

/// EratSmall is an implementation of the segmented sieve
/// of Eratosthenes optimized for small sieving primes that
/// have many multiples per segment.
///
class EratSmall : public Wheel30_t
{
public:
  static uint64_t getL1CacheSize(uint64_t);
  void init(uint64_t, uint64_t, uint64_t);
  void crossOff(uint8_t*, uint64_t);
  bool hasSievingPrimes() const { return !primes_.empty(); }
private:
  uint64_t maxPrime_ = 0;
  uint64_t l1CacheSize_ = 0;
  std::vector<SievingPrime> primes_;
  void storeSievingPrime(uint64_t, uint64_t, uint64_t);
  NOINLINE void crossOff(uint8_t*, uint8_t*);
};

} // namespace

#endif
