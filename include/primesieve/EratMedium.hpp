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
#include "Wheel.hpp"
#include "types.hpp"

#include <stdint.h>
#include <vector>

namespace primesieve {

/// EratMedium is an implementation of the segmented sieve
/// of Eratosthenes optimized for medium sieving primes that
/// have a few multiples per segment
///
class EratMedium : public Wheel210_t
{
public:
  void init(uint64_t, uint64_t, uint64_t);
  void crossOff(byte_t*, uint64_t);
  bool enabled() const { return enabled_; }
private:
  uint64_t maxPrime_;
  std::vector<SievingPrime> primes_;
  bool enabled_ = false;
  void storeSievingPrime(uint64_t, uint64_t, uint64_t);
};

} // namespace

#endif
