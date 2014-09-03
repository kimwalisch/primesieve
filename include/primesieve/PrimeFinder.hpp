///
/// @file  PrimeFinder.hpp
///
/// Copyright (C) 2014 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRIMEFINDER_HPP
#define PRIMEFINDER_HPP

#include "config.hpp"
#include "SieveOfEratosthenes.hpp"

#include <stdint.h>
#include <vector>

namespace primesieve {

class PrimeSieve;

/// PrimeFinder is a SieveOfEratosthenes class that is used to
/// callback, print and count primes and prime k-tuplets
/// (twin primes, prime triplets, ...).
///
class PrimeFinder : public SieveOfEratosthenes {
public:
  PrimeFinder(PrimeSieve&);
private:
  enum { END = 0xff + 1 };
  static const uint_t kBitmasks_[6][5];
  /// Count lookup tables for prime k-tuplets
  std::vector<uint_t> kCounts_[6];
  /// Reference to the associated PrimeSieve object
  PrimeSieve& ps_;
  void init_kCounts();
  virtual void segmentFinished(const byte_t*, uint_t);
  void count(const byte_t*, uint_t);
  void print(const byte_t*, uint_t) const;
  template <typename T> void callbackPrimes(T, const byte_t*, uint_t) const;
  template <typename T> void callbackPrimes(T, const byte_t*, uint_t, int) const;
  void callbackPrimes(const byte_t*, uint_t) const;
  static void printPrime(uint64_t);
  DISALLOW_COPY_AND_ASSIGN(PrimeFinder);
};

} // namespace primesieve

#endif
