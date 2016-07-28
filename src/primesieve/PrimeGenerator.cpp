///
/// @file  PrimeGenerator.cpp
///        Generates the sieving primes up to sqrt(stop) and adds
///        them to PrimeFinder.
///
/// Copyright (C) 2016 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/config.hpp>
#include <primesieve/PrimeGenerator.hpp>
#include <primesieve/PrimeFinder.hpp>
#include <primesieve/PreSieve.hpp>
#include <primesieve/SieveOfEratosthenes.hpp>
#include <primesieve/SieveOfEratosthenes-inline.hpp>
#include <primesieve/littleendian_cast.hpp>

#include <stdint.h>
#include <vector>

namespace primesieve {

PrimeGenerator::PrimeGenerator(PrimeFinder& finder, const PreSieve& preSieve) :
  SieveOfEratosthenes(preSieve.getLimit() + 1,
                      finder.getSqrtStop(),
                      config::PRIMEGENERATOR_SIEVESIZE,
                      preSieve),
  finder_(finder)
{ }

void PrimeGenerator::generateSievingPrimes()
{
  generateTinyPrimes();
  // calls segmentFinished() when done
  sieve();
}

/// Generate the primes up to finder.getSqrtStop()
/// using the sieve of Eratosthenes.
///
void PrimeGenerator::generateTinyPrimes()
{
  uint_t s = (uint_t) getStart();
  uint_t n = getSqrtStop();

  std::vector<char> isPrime(n + 1, true);

  for (uint_t i = 3; i * i <= n; i += 2)
    if (isPrime[i])
      for (uint_t j = i * i; j <= n; j += i * 2)
        isPrime[j] = false;

  // make sure s is odd
  s += (~s & 1);
  for (uint_t i = s; i <= n; i += 2)
    if (isPrime[i])
      addSievingPrime(i);
}

void PrimeGenerator::segmentFinished(const byte_t* sieve, uint_t sieveSize)
{
  generateSievingPrimes(sieve, sieveSize);
}

/// Reconstruct primes from 1 bits of the sieve array
/// and use them for sieving in finder_.
///
void PrimeGenerator::generateSievingPrimes(const byte_t* sieve, uint_t sieveSize)
{
  uint64_t base = getSegmentLow();

  for (uint_t i = 0; i < sieveSize; i += 8)
  {
    uint64_t bits = littleendian_cast<uint64_t>(&sieve[i]);

    while (bits != 0)
      finder_.addSievingPrime((uint_t) getNextPrime(&bits, base));

    base += NUMBERS_PER_BYTE * 8;
  }
}

} // namespace primesieve
