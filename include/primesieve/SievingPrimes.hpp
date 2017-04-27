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

class PrimeFinder;
class PreSieve;

class SievingPrimes : public SieveOfEratosthenes
{
public:
  SievingPrimes(PrimeFinder&, const PreSieve&);
  void generateSievingPrimes();
private:
  PrimeFinder& finder_;
  void segmentFinished(const byte_t*, uint_t);
  void generateTinyPrimes();
};

} // namespace

#endif
