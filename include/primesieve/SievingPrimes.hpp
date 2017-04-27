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

namespace primesieve {

class PrimeGenerator;
class PreSieve;

class SievingPrimes : public SieveOfEratosthenes
{
public:
  SievingPrimes(PrimeGenerator&, const PreSieve&);
  void generate();
private:
  PrimeGenerator& primeGen_;
  void generatePrimes(const byte_t*, uint_t);
  void tinyPrimes();
};

} // namespace

#endif
