///
/// @file  EratMedium.h
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef ERATMEDIUM_H
#define ERATMEDIUM_H

#include "config.h"
#include "WheelFactorization.h"

#include <stdint.h>
#include <list>

namespace soe {

/// EratMedium is an implementation of the segmented sieve of
/// Eratosthenes optimized for medium sieving primes that have a few
/// multiples per segment.
///
class EratMedium : public Modulo210Wheel_t {
public:
  EratMedium(uint64_t, uint_t, uint_t);
  uint_t getLimit() const { return limit_; }
  void crossOff(byte_t*, uint_t);
private:
  typedef std::list<Bucket>::iterator BucketIterator_t;
  const uint_t limit_;
  /// List of buckets, holds the sieving primes
  std::list<Bucket> buckets_;
  void storeSievingPrime(uint_t, uint_t, uint_t);
  static void crossOff(byte_t*, uint_t, Bucket&);
  DISALLOW_COPY_AND_ASSIGN(EratMedium);
};

} // namespace soe

#endif
