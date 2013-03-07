///
/// @file  EratBig.h
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef ERATBIG_H
#define ERATBIG_H

#include "config.h"
#include "WheelFactorization.h"

#include <stdint.h>
#include <vector>
#include <list>

namespace soe {

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
  typedef std::list<Bucket*>::iterator PointerIterator_t;
  const uint_t limit_;
  /// log2 of SieveOfEratosthenes::sieveSize_
  const uint_t log2SieveSize_;
  const uint_t moduloSieveSize_;
  /// Vector of bucket lists, holds the sieving primes
  std::vector<Bucket*> lists_;
  /// List of empty buckets
  Bucket* stock_;
  /// Pointers of the allocated buckets
  std::list<Bucket*> pointers_;
  void init(uint_t);
  static void moveBucket(Bucket&, Bucket*&);
  void pushBucket(uint_t);
  void storeSievingPrime(uint_t, uint_t, uint_t);
  void crossOff(byte_t*, Bucket&);
  uint_t getSegment(uint_t*);
  DISALLOW_COPY_AND_ASSIGN(EratBig);
};

} // namespace soe

#endif
