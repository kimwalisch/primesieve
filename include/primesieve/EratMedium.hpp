///
/// @file  EratMedium.hpp
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef ERATMEDIUM_HPP
#define ERATMEDIUM_HPP

#include "config.hpp"
#include "Wheel.hpp"

#include <stdint.h>
#include <vector>

namespace primesieve {

/// EratMedium is an implementation of the segmented sieve
/// of Eratosthenes optimized for medium sieving primes that
/// have a few multiples per segment
///
class EratMedium : public Modulo210Wheel_t
{
public:
  EratMedium(uint64_t, uint_t, uint_t);
  void crossOff(byte_t*, uint_t);
private:
  uint_t maxPrime_;
  std::vector<SievingPrime> primes_;
  void storeSievingPrime(uint_t, uint_t, uint_t);
};

} // namespace

#endif
