///
/// @file  PrintPrimes.hpp
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRINTPRIMES_HPP
#define PRINTPRIMES_HPP

#include "Erat.hpp"
#include "PrimeSieve.hpp"
#include "types.hpp"

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
  uint64_t low_ = 0;
  /// Count lookup tables for prime k-tuplets
  std::vector<byte_t> kCounts_[6];
  counts_t& counts_;
  /// Reference to the associated PrimeSieve object
  PrimeSieve& ps_;
  void initCounts();
  void print();
  void countPrimes();
  void countkTuplets();
  void printPrimes() const;
  void printkTuplets() const;
};

} // namespace

#endif
