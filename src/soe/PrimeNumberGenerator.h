///
/// @file  PrimeNumberGenerator.h
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the New BSD License. See the
/// LICENSE file in the top level directory.
///

#ifndef PRIMENUMBERGENERATOR_H
#define PRIMENUMBERGENERATOR_H

#include "config.h"
#include "SieveOfEratosthenes.h"

namespace soe {

class PrimeFinder;

class PrimeNumberGenerator : public SieveOfEratosthenes {
public:
  PrimeNumberGenerator(PrimeFinder&);
  void doIt();
private:
  PrimeFinder& finder_;
  void segmentProcessed(const byte_t*, uint_t);
  DISALLOW_COPY_AND_ASSIGN(PrimeNumberGenerator);
};

} // namespace soe

#endif
