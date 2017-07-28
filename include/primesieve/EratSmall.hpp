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
#include "Wheel.hpp"

#include <stdint.h>
#include <cstddef>
#include <vector>

namespace primesieve {

/// EratSmall is an implementation of the segmented sieve
/// of Eratosthenes optimized for small sieving primes that
/// have many multiples per segment
///
class EratSmall : public Modulo30Wheel_t
{
public:
  EratSmall(uint64_t, uint_t, uint_t);
  void crossOff(byte_t*, uint_t);
private:
  uint_t maxPrime_;
  size_t l1CacheSize_;
  std::vector<SievingPrime> primes_;
  void storeSievingPrime(uint_t, uint_t, uint_t);
  void crossOff(byte_t*, byte_t*);
};

} // namespace

#endif
