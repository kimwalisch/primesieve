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
#include <list>

namespace primesieve {

/// EratMedium is an implementation of the segmented sieve
/// of Eratosthenes optimized for medium sieving primes that
/// have a few multiples per segment
///
class EratMedium : public Modulo210Wheel_t
{
public:
  EratMedium(uint64_t, uint_t, uint_t);
  uint_t getLimit() const { return limit_; }
  void crossOff(byte_t*, uint_t);
private:
  uint_t limit_;
  std::list<Bucket> buckets_;
  void storeSievingPrime(uint_t, uint_t, uint_t);
  static void crossOff(byte_t*, uint_t, Bucket&);
};

} // namespace

#endif
