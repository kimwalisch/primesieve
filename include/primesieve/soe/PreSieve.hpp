///
/// @file  PreSieve.hpp
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRESIEVE_HPP
#define PRESIEVE_HPP

#include "config.hpp"
#include <stdint.h>

namespace primesieve {

/// @brief  Pre-sieve multiples of small primes to speed up the sieve
///         of Eratosthenes.
///
/// PreSieve objects are used to pre-sieve multiples of small primes
/// e.g. <= 19 to speed up SieveOfEratosthenes. The idea is to
/// allocate an array (preSieved_) and remove the multiples of small
/// primes from it at initialization. Then whilst sieving, the
/// preSieved_ array is copied to the SieveOfEratosthenes array at the
/// beginning of each new segment to pre-sieve the multiples of small
/// primes <= limit_. Pre-sieving speeds up my sieve of Eratosthenes
/// implementation by about 20 percent when sieving < 10^10.
///
/// <b> Memory Usage </b>
///
/// - PreSieve objects use: primeProduct(limit_) / 30 bytes of memory
/// - PreSieve multiples of primes <= 11 uses   77    bytes
/// - PreSieve multiples of primes <= 13 uses 1001    bytes
/// - PreSieve multiples of primes <= 17 uses   16.62 kilobytes
/// - PreSieve multiples of primes <= 19 uses  315.75 kilobytes
/// - PreSieve multiples of primes <= 23 uses    7.09 megabytes
///
class PreSieve {
public:
  PreSieve(int);
  ~PreSieve();
  uint_t getLimit() const { return limit_; }
  void doIt(byte_t*, uint_t, uint64_t) const;
private:
  static const uint_t primes_[10];
  /// Pre-sieve multiples of primes <= limit_ (>= 11 && <= 23)
  uint_t limit_;
  uint_t primeProduct_;
  byte_t* preSieved_;
  uint_t size_;
  void init();
  DISALLOW_COPY_AND_ASSIGN(PreSieve);
};

} // namespace primesieve

#endif
