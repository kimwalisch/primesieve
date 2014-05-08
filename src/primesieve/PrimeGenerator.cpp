///
/// @file  PrimeGenerator.cpp
///        Generates the sieving primes up to sqrt(stop) and adds
///        them to PrimeFinder.
///
/// Copyright (C) 2014 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/config.hpp>
#include <primesieve/PrimeGenerator.hpp>
#include <primesieve/PrimeFinder.hpp>
#include <primesieve/SieveOfEratosthenes.hpp>
#include <primesieve/SieveOfEratosthenes-inline.hpp>
#include <primesieve/littleendian_cast.hpp>

#include <vector>
#include <cassert>

namespace primesieve {

PrimeGenerator::PrimeGenerator(PrimeFinder& finder) :
  SieveOfEratosthenes(finder.getPreSieve() + 1,
                      finder.getSqrtStop(),
                      config::PRIMEGENERATOR_SIEVESIZE),
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
  for (uint_t i = P + (~P & 1); i <= N; i += 2)
    if (isPrime[i])
      addSievingPrime(i);
}

void PrimeGenerator::doIt()
{
  generateTinyPrimes();
  // calls segmentFinished() after each sieved segment
  sieve();
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
    {
      uint_t prime = static_cast<uint_t>(getNextPrime(&bits, base));
      finder_.addSievingPrime(prime);
    }
    base += NUMBERS_PER_BYTE * 8;
  }
}

} // namespace primesieve
