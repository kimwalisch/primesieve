///
/// @file  PrimeGenerator.cpp
///        Generates the sieving primes up to sqrt(stop) and adds
///        them to PrimeFinder (finder_).
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include "config.h"
#include "PrimeGenerator.h"
#include "PrimeFinder.h"
#include "SieveOfEratosthenes.h"
#include "SieveOfEratosthenes-GENERATE.h"
#include "SieveOfEratosthenes-inline.h"

#include <vector>

namespace soe {

PrimeGenerator::PrimeGenerator(PrimeFinder& finder) :
  SieveOfEratosthenes(finder.getPreSieve() + 1,
                      finder.getSqrtStop(),
                      config::SIEVESIZE),
  finder_(finder)
{ }

void PrimeGenerator::doIt()
{
  // tiny sieve of Eratosthenes that generates the sieving
  // primes up to sqrt( sqrt(stop) )
  uint_t N = getSqrtStop();
  std::vector<byte_t> isPrime(N / 8 + 1, 0xAA);
  for (uint_t i = 3; i * i <= N; i += 2) {
    if (isPrime[i >> 3] & (1 << (i & 7)))
      for (uint_t j = i * i; j <= N; j += i * 2)
        isPrime[j >> 3] &= ~(1 << (j & 7));
  }
  // add primes to this PrimeGenerator
  for (uint_t i = getPreSieve() + 1; i <= N; i++) {
    if (isPrime[i >> 3] & (1 << (i & 7)))
      addSievingPrime(i);
  }
  // sieve up to sqrt(stop)
  sieve();
}

void PrimeGenerator::segmentFinished(const byte_t* sieve, uint_t sieveSize)
{
  generate(sieve, sieveSize);
}

/// Reconstruct the primes from 1 bits of the sieve
/// array and add them to finder_.
///
void PrimeGenerator::generate(const byte_t* sieve, uint_t sieveSize)
{
  GENERATE_PRIMES(finder_.addSievingPrime, uint_t)
}

} // namespace soe
