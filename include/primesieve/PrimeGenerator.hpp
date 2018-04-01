///
/// @file  PrimeGenerator.hpp
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRIMEGENERATOR_HPP
#define PRIMEGENERATOR_HPP

#include "config.hpp"
#include "Erat.hpp"
#include "PrimeSieve.hpp"

#include <stdint.h>
#include <vector>

namespace primesieve {

class PreSieve;
class Store;

/// After a segment has been sieved PrimeGenerator is
/// used to reconstruct primes and prime k-tuplets from
/// 1 bits of the sieve array
///
class PrimeGenerator : public Erat
{
public:
  PrimeGenerator(PrimeSieve&, const PreSieve&);
  void sieve();
private:
  enum { END = 0xff + 1 };
  static const uint64_t bitmasks_[6][5];
  /// Count lookup tables for prime k-tuplets
  std::vector<byte_t> kCounts_[6];
  /// Reference to the associated PrimeSieve object
  PrimeSieve& ps_;
  PrimeSieve::counts_t& counts_;
  void init_kCounts();
  virtual void generatePrimes(const byte_t*, uint64_t);
  void count(const byte_t*, uint64_t);
  void print(const byte_t*, uint64_t) const;
  void storePrimes(Store&, const byte_t*, uint64_t) const;
  static void printPrime(uint64_t);
};

} // namespace

#endif
