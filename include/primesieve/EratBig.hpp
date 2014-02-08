///
/// @file  EratBig.hpp
///
/// Copyright (C) 2014 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef ERATBIG_HPP
#define ERATBIG_HPP

#include "config.hpp"
#include "WheelFactorization.hpp"

#include <stdint.h>
#include <vector>

namespace primesieve {

/// EratBig is an implementation of the segmented sieve of
/// Eratosthenes optimized for big sieving primes that have very few
/// multiples per segment.
///
class EratBig: public Modulo210Wheel_t {
public:
  EratBig(uint64_t, uint_t, uint_t);
  ~EratBig();
  void crossOff(byte_t*);
private:
  const uint_t limit_;
  /// log2 of SieveOfEratosthenes::sieveSize_
  const uint_t log2SieveSize_;
  const uint_t moduloSieveSize_;
  /// Vector of bucket lists, holds the sieving primes
  std::vector<Bucket*> lists_;
  /// List of empty buckets
  Bucket* stock_;
  /// Pointers of the allocated buckets
  std::vector<Bucket*> pointers_;
  void init(uint_t);
  static void moveBucket(Bucket&, Bucket*&);
  void pushBucket(uint_t);
  void storeSievingPrime(uint_t, uint_t, uint_t);
  void crossOff(byte_t*, SievingPrime*, SievingPrime*);
  DISALLOW_COPY_AND_ASSIGN(EratBig);
};

} // namespace primesieve

#endif
