///
/// @file  PrimeNumberFinder.h
///
/// Copyright (C) 2012 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the New BSD License. See the
/// LICENSE file in the top level directory.
///

#ifndef PRIMENUMBERFINDER_H
#define PRIMENUMBERFINDER_H

#include "config.h"
#include "SieveOfEratosthenes.h"

#include <stdint.h>
#include <vector>

class PrimeSieve;

namespace soe {

/// PrimeNumberFinder is a SieveOfEratosthenes class that is used to
/// generate, count and print primes and prime k-tuplets
/// (twin primes, prime triplets, ...).
///
class PrimeNumberFinder : public SieveOfEratosthenes {
public:
  PrimeNumberFinder(PrimeSieve&);
private:
  enum { END = 0xff + 1 };
  static const uint_t kBitmasks_[7][5];
  /// Reference to the friend PrimeSieve object
  PrimeSieve& ps_;
  /// Lookup tables that give the count of prime k-tuplets
  /// (twin primes, prime triplets, ...) per byte.
  std::vector<uint_t> kCounts_[7];
  void init_kCounts();
  virtual void segmentProcessed(const uint8_t*, uint_t);
  void count(const uint8_t*, uint_t);
  void generate(const uint8_t*, uint_t) const;
  void callback32_obj(uint32_t) const;
  void callback64_obj(uint64_t) const;
  void callback64_int(uint64_t) const;
  static void print(uint64_t);
  DISALLOW_COPY_AND_ASSIGN(PrimeNumberFinder);
};

} // namespace soe

#endif
