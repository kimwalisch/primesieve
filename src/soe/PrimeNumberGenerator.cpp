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

#include <vector>
#include <stdint.h>

namespace soe {

PrimeNumberGenerator::PrimeNumberGenerator(PrimeNumberFinder& finder) :
  SieveOfEratosthenes(finder.getPreSieve() + 1,
                      finder.getSqrtStop(),
                      config::SIEVESIZE,
                      config::PRESIEVE_GENERATOR),
  finder_(finder)
{ }

/// Generate the primes up to finder_.getStop()**0.5
/// and use them to sieve with finder_.
///
void PrimeNumberGenerator::doIt()
{
  // tiny sieve of Eratosthenes that generates the primes
  // up to finder_.getStop()**0.25
  uint_t N = getSqrtStop();
  std::vector<uint8_t> isPrime(N / 8 + 1, 0xAA);
  for (uint_t i = 3; i * i <= N; i += 2) {
    if (isPrime[i >> 3] & (1 << (i & 7)))
      for (uint_t j = i * i; j <= N; j += i + i)
        isPrime[j >> 3] &= ~(1 << (j & 7));
  }
  for (uint_t i = getPreSieve() + 1; i <= N; i++) {
    if (isPrime[i >> 3] & (1 << (i & 7)))
      sieve(i);
  }
  finish();
}

/// Executed after each sieved segment, generates the primes
/// within the current segment and uses them to
/// sieve with finder_.
///
void PrimeNumberGenerator::segmentProcessed(const uint8_t* sieve, uint_t sieveSize)
{
  GENERATE_PRIMES(finder_.sieve, uint_t)
}

} // namespace soe
