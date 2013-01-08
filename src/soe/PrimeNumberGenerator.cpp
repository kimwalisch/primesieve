///
/// @file  PrimeNumberGenerator.cpp
///        PrimeNumberGenerator generates the primes up to sqrt(n)
///        needed for sieving by PrimeNumberFinder.
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the New BSD License. See the
/// LICENSE file in the top level directory.
///

#include "config.h"
#include "PrimeNumberGenerator.h"
#include "PrimeNumberFinder.h"
#include "SieveOfEratosthenes.h"
#include "SieveOfEratosthenes-GENERATE.h"
#include "SieveOfEratosthenes-inline.h"

#include <stdint.h>

namespace soe {

PrimeNumberGenerator::PrimeNumberGenerator(PrimeNumberFinder& finder) :
  SieveOfEratosthenes(finder.getPreSieve() + 1,
                      finder.getSqrtStop(),
                      config::SIEVESIZE,
                      config::PRESIEVE_GENERATOR),
  finder_(finder)
{ }

/// Generate the primes within the current segment
/// and use them to sieve with finder_.
///
void PrimeNumberGenerator::segmentProcessed(const uint8_t* sieve, uint_t sieveSize)
{
  GENERATE_PRIMES(finder_.sieve, uint_t)
}

} // namespace soe
