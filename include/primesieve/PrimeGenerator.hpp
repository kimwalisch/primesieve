///
/// @file  PrimeGenerator.hpp
///
/// Copyright (C) 2014 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRIMEGENERATOR_HPP
#define PRIMEGENERATOR_HPP

#include "config.hpp"
#include "SieveOfEratosthenes.hpp"

namespace primesieve {

class PrimeFinder;

class PrimeGenerator : public SieveOfEratosthenes {
public:
  PrimeGenerator(PrimeFinder&);
  void doIt();
private:
  PrimeFinder& finder_;
  void segmentFinished(const byte_t*, uint_t);
  void generateSievingPrimes(const byte_t*, uint_t);
  void generateTinyPrimes();
  DISALLOW_COPY_AND_ASSIGN(PrimeGenerator);
};

} // namespace primesieve

#endif
