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
#include <list>

namespace primesieve {

/// EratSmall is an implementation of the segmented sieve
/// of Eratosthenes optimized for small sieving primes that
/// have many multiples per segment
///
class EratSmall : public Modulo30Wheel_t
{
public:
  EratSmall(uint64_t, uint_t, uint_t);
  uint_t getLimit() const { return limit_; }
  void crossOff(byte_t*, byte_t*);
private:
  uint_t limit_;
  std::list<Bucket> buckets_;
  void storeSievingPrime(uint_t, uint_t, uint_t);
  static void crossOff(byte_t*, byte_t*, Bucket&);
};

} // namespace

#endif
