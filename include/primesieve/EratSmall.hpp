///
/// @file  EratSmall.hpp
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef ERATSMALL_HPP
#define ERATSMALL_HPP

#include "config.hpp"
#include "Bucket.hpp"
#include "Wheel.hpp"

#include <stdint.h>
#include <cstddef>
#include <vector>

namespace primesieve {

/// EratSmall is an implementation of the segmented sieve
/// of Eratosthenes optimized for small sieving primes that
/// have many multiples per segment
///
class EratSmall : public Wheel30_t
{
public:
  EratSmall(uint64_t, uint64_t, uint64_t);
  static uint64_t getL1Size(uint64_t);
  void crossOff(byte_t*, uint64_t);
private:
  uint64_t maxPrime_;
  uint64_t l1Size_;
  std::vector<SievingPrime> primes_;
  void storeSievingPrime(uint64_t, uint64_t, uint64_t);
  void crossOff(byte_t*, byte_t*);
};

} // namespace

#endif
