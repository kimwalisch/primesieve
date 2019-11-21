///
/// @file  PreSieve.hpp
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRESIEVE_HPP
#define PRESIEVE_HPP

#include <stdint.h>
#include <memory>

namespace primesieve {

/// PreSieve objects are used to pre-sieve multiples of small primes
/// e.g. <= 19 to speed up the sieve of Eratosthenes. The idea is to
/// allocate an array (buffer_) and remove the multiples of small
/// primes from it at initialization. Then whilst sieving, the
/// buffer_ array is copied to the sieve array at the beginning of
/// each new segment to pre-sieve the multiples of small
/// primes <= maxPrime_. Pre-sieving speeds up my sieve of Eratosthenes
/// implementation by about 20 percent when sieving < 10^10.
///
/// <b> Memory Usage </b>
///
/// - PreSieve objects use: primeProduct(maxPrime_) / 30 bytes of memory
/// - PreSieve multiples of primes <=  7 uses    7    bytes
/// - PreSieve multiples of primes <= 11 uses   77    bytes
/// - PreSieve multiples of primes <= 13 uses 1001    bytes
/// - PreSieve multiples of primes <= 17 uses   17.02 kilobytes
/// - PreSieve multiples of primes <= 19 uses  323.32 kilobytes
/// - PreSieve multiples of primes <= 23 uses    7.44 megabytes
///
class PreSieve
{
public:
  void init(uint64_t, uint64_t);
  uint64_t getMaxPrime() const { return maxPrime_; }
  void copy(uint8_t*, uint64_t, uint64_t) const;
private:
  uint64_t maxPrime_ = 0;
  uint64_t primeProduct_ = 0;
  uint64_t size_ = 0;
  uint8_t* buffer_ = nullptr;
  std::unique_ptr<uint8_t[]> deleter_;
  void initBuffer(uint64_t, uint64_t);
};

} // namespace

#endif
