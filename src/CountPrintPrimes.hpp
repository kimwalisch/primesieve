///
/// @file  CountPrintPrimes.hpp
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef COUNTPRINTPRIMES_HPP
#define COUNTPRINTPRIMES_HPP

#include "Erat.hpp"
#include "MemoryPool.hpp"
#include "PrimeSieveClass.hpp"

#include <primesieve/macros.hpp>
#include <primesieve/Vector.hpp>

#include <stdint.h>

namespace primesieve {

class Store;

/// After a segment has been sieved CountPrintPrimes is
/// used to reconstruct primes and prime k-tuplets from
/// 1 bits of the sieve array.
///
class CountPrintPrimes : public Erat
{
public:
  CountPrintPrimes(PrimeSieve&);
  NOINLINE void sieve();
private:
  uint64_t low_ = 0;
  /// Count lookup tables for prime k-tuplets
  Vector<uint8_t> kCounts_[6];
  counts_t& counts_;
  /// Reference to the associated PrimeSieve object
  PrimeSieve& ps_;
  MemoryPool memoryPool_;
  void initCounts();
  void countPrimes();
  void countkTuplets();
  void printPrimes() const;
  void printkTuplets() const;
};

} // namespace

#endif
