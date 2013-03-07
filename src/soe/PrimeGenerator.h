///
/// @file  PrimeGenerator.h
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRIMEGENERATOR_H
#define PRIMEGENERATOR_H

#include "config.h"
#include "SieveOfEratosthenes.h"

namespace soe {

class PrimeFinder;

class PrimeGenerator : public SieveOfEratosthenes {
public:
  PrimeGenerator(PrimeFinder&);
  void doIt();
private:
  PrimeFinder& finder_;
  void segmentFinished(const byte_t*, uint_t);
  void generate(const byte_t*, uint_t);
  DISALLOW_COPY_AND_ASSIGN(PrimeGenerator);
};

} // namespace soe

#endif
