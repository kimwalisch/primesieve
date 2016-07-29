///
/// @file  PreSieve.hpp
///
/// Copyright (C) 2016 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRESIEVE_HPP
#define PRESIEVE_HPP

#include "config.hpp"
#include <stdint.h>

namespace primesieve {

/// PreSieve objects are used to pre-sieve multiples of small primes
/// e.g. <= 19 to speed up SieveOfEratosthenes. The idea is to
/// allocate an array (preSieve_) and remove the multiples of small
/// primes from it at initialization. Then whilst sieving, the
/// preSieve_ array is copied to the SieveOfEratosthenes array at the
/// beginning of each new segment to pre-sieve the multiples of small
/// primes <= limit_. Pre-sieving speeds up my sieve of Eratosthenes
/// implementation by about 20 percent when sieving < 10^10.
///
/// <b> Memory Usage </b>
///
/// - PreSieve objects use: primeProduct(limit_) / 30 bytes of memory
/// - PreSieve multiples of primes <=  7 uses    7    bytes
/// - PreSieve multiples of primes <= 11 uses   77    bytes
/// - PreSieve multiples of primes <= 13 uses 1001    bytes
/// - PreSieve multiples of primes <= 17 uses   16.62 kilobytes
/// - PreSieve multiples of primes <= 19 uses  315.75 kilobytes
/// - PreSieve multiples of primes <= 23 uses    7.09 megabytes
///
class PreSieve {
public:
  PreSieve(uint64_t start, uint64_t stop);
  ~PreSieve();
  uint_t getLimit() const { return limit_; }
  void doIt(byte_t* sieve, uint_t sieveSize, uint64_t segmentLow) const;
private:
  uint_t limit_;
  uint_t primeProduct_;
  byte_t* preSieve_;
  uint_t size_;
  void init();
  DISALLOW_COPY_AND_ASSIGN(PreSieve);
};

} // namespace

#endif
