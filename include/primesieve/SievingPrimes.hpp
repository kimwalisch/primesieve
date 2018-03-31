///
/// @file  SievingPrimes.hpp
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef SIEVINGPRIMES_HPP
#define SIEVINGPRIMES_HPP

#include "config.hpp"
#include "SieveOfEratosthenes.hpp"

#include <vector>

namespace primesieve {

class PrimeGenerator;
class PreSieve;

class SievingPrimes : public SieveOfEratosthenes
{
public:
  SievingPrimes(PrimeGenerator&, const PreSieve&);
  uint64_t next_prime();
private:
  PrimeGenerator& primeGen_;
  std::vector<char> isPrime_;
  uint64_t num_;
  uint64_t low_;
  uint64_t tinyIdx_;
  uint64_t sieveIdx_;
  uint64_t primes_[64];

  void generatePrimes(const byte_t*, uint64_t) { }
  void fill();
  void sieveOneSegment();
  void tinyPrimes();
};

} // namespace

#endif
