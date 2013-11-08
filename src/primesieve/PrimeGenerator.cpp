///
/// @file  PrimeGenerator.cpp
///        Generates the sieving primes up to sqrt(stop) and adds
///        them to PrimeFinder.
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/config.hpp>
#include <primesieve/PrimeGenerator.hpp>
#include <primesieve/PrimeFinder.hpp>
#include <primesieve/SieveOfEratosthenes.hpp>
#include <primesieve/SieveOfEratosthenes-CALLBACK.hpp>
#include <primesieve/SieveOfEratosthenes-inline.hpp>

#include <vector>
#include <cassert>

namespace primesieve {

PrimeGenerator::PrimeGenerator(PrimeFinder& finder) :
  SieveOfEratosthenes(finder.getPreSieve() + 1,
                      finder.getSqrtStop(),
                      config::SIEVESIZE),
  finder_(finder)
{ }

/// Generate the primes up to finder.stop_^0.25 using
/// the sieve of Eratosthenes.
///
void PrimeGenerator::generateTinyPrimes()
{
  uint_t P = getPreSieve() + 1;
  uint_t N = getSqrtStop();
  std::vector<char> isPrime(N + 1, true);

  for (uint_t i = 3; i * i <= N; i += 2)
    if (isPrime[i])
      for (uint_t j = i * i; j <= N; j += i * 2)
        isPrime[j] = false;

  assert(P > 5);
  for (uint_t i = P + ~P % 2; i <= N; i += 2)
    if (isPrime[i])
      addSievingPrime(i);
}

void PrimeGenerator::doIt()
{
  generateTinyPrimes();
  sieve();
}

void PrimeGenerator::segmentFinished(const byte_t* sieve, uint_t sieveSize)
{
  callback(sieve, sieveSize);
}

/// Reconstruct the primes from 1 bits of the sieve
/// array and add them to finder_.
///
void PrimeGenerator::callback(const byte_t* sieve, uint_t sieveSize)
{
  CALLBACK_PRIMES(finder_.addSievingPrime, uint_t)
}

} // namespace primesieve
