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

namespace soe {

PrimeNumberGenerator::PrimeNumberGenerator(PrimeNumberFinder& finder) :
  SieveOfEratosthenes(finder.getPreSieve() + 1,
                      finder.getSqrtStop(),
                      config::SIEVESIZE,
                      config::PRESIEVE_GENERATOR),
  finder_(finder)
{ }

/// Generate the primes up to finder_.getStop()^0.5
/// and add them to finder_.
///
void PrimeNumberGenerator::doIt()
{
  // first generate the sieving primes up to finder_.getStop()^0.25
  uint_t N = getSqrtStop();
  std::vector<byte_t> isPrime(N / 8 + 1, 0xAA);
  for (uint_t i = 3; i * i <= N; i += 2) {
    if (isPrime[i >> 3] & (1 << (i & 7)))
      for (uint_t j = i * i; j <= N; j += i * 2)
        isPrime[j >> 3] &= ~(1 << (j & 7));
  }
  for (uint_t i = getPreSieve() + 1; i <= N; i++) {
    if (isPrime[i >> 3] & (1 << (i & 7)))
      addSievingPrime(i);
  }
  // second generate the primes up to finder_.getStop()^0.5
  sieve();
}

/// Reconstruct the primes from 1 bits of the sieve
/// array and add them to finder_.
///
void PrimeNumberGenerator::segmentProcessed(const byte_t* sieve, uint_t sieveSize)
{
  GENERATE_PRIMES(finder_.addSievingPrime, uint_t)
}

} // namespace soe
