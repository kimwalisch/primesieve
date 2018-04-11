///
/// @file  PrintPrimes.hpp
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRIMEGENERATOR_HPP
#define PRIMEGENERATOR_HPP

#include "config.hpp"
#include "Erat.hpp"
#include "PrimeSieve.hpp"
#include "PreSieve.hpp"

#include <stdint.h>
#include <vector>

namespace primesieve {

class Store;

/// After a segment has been sieved PrintPrimes is
/// used to reconstruct primes and prime k-tuplets from
/// 1 bits of the sieve array
///
class PrintPrimes : public Erat
{
public:
  PrintPrimes(PrimeSieve&);
  void sieve();
private:
  enum { END = 0xff + 1 };
  static const uint64_t bitmasks_[6][5];
  /// Count lookup tables for prime k-tuplets
  std::vector<byte_t> kCounts_[6];
  /// Reference to the associated PrimeSieve object
  PreSieve preSieve_;
  PrimeSieve::counts_t& counts_;
  PrimeSieve& ps_;
  void initCounts();
  void generatePrimes(const byte_t*, uint64_t);
  void countPrimes(const byte_t*, uint64_t);
  void countkTuplets(const byte_t*, uint64_t);
  void printPrimes(const byte_t*, uint64_t) const;
  void printkTuplets(const byte_t*, uint64_t) const;
};

} // namespace

#endif
