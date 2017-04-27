///
/// @file  EratBig.hpp
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef ERATBIG_HPP
#define ERATBIG_HPP

#include "config.hpp"
#include "Wheel.hpp"

#include <stdint.h>
#include <memory>
#include <vector>

namespace primesieve {

/// EratBig is an implementation of the segmented sieve of
/// Eratosthenes optimized for big sieving primes that have
/// very few multiples per segment
///
class EratBig : public Modulo210Wheel_t
{
public:
  EratBig(uint64_t, uint_t, uint_t);
  void crossOff(byte_t*);
private:
  uint_t limit_;
  /// log2 of SieveOfEratosthenes::sieveSize_
  uint_t log2SieveSize_;
  uint_t moduloSieveSize_;
  /// Vector of bucket lists, holds the sieving primes
  std::vector<Bucket*> lists_;
  /// List of empty buckets
  Bucket* stock_;
  /// Pointers of the allocated buckets
  std::vector<std::unique_ptr<Bucket[]>> memory_;
  void init(uint_t);
  static void moveBucket(Bucket&, Bucket*&);
  void pushBucket(uint_t);
  void storeSievingPrime(uint_t, uint_t, uint_t);
  void crossOff(byte_t*, SievingPrime*, SievingPrime*);
};

} // namespace

#endif
