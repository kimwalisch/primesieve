///
/// @file  SievingPrimes.cpp
///        Generates the sieving primes up to sqrt(stop)
///        and adds them to PrimeGenerator.
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/config.hpp>
#include <primesieve/SievingPrimes.hpp>
#include <primesieve/PrimeGenerator.hpp>
#include <primesieve/PreSieve.hpp>
#include <primesieve/SieveOfEratosthenes.hpp>
#include <primesieve/littleendian_cast.hpp>

#include <stdint.h>
#include <vector>

using namespace std;

namespace primesieve {

SievingPrimes::SievingPrimes(PrimeGenerator& primeGen, const PreSieve& preSieve) :
  SieveOfEratosthenes(preSieve.getLimit() + 1,
                      primeGen.getSqrtStop(),
                      L1_DCACHE_SIZE,
                      preSieve),
  primeGen_(primeGen)
{ }

void SievingPrimes::generate()
{
  tinyPrimes();
  sieve();
}

/// Generate the primes up to sqrt(sqrt(stop))
/// using the sieve of Eratosthenes
///
void SievingPrimes::tinyPrimes()
{
  uint_t n = getSqrtStop();
  vector<char> isPrime(n + 1, true);

  for (uint_t i = 3; i * i <= n; i += 2)
    if (isPrime[i])
      for (uint_t j = i * i; j <= n; j += i * 2)
        isPrime[j] = false;

  uint_t s = (uint_t) getStart();
  s += (~s & 1);

  for (uint_t i = s; i <= n; i += 2)
    if (isPrime[i])
      addSievingPrime(i);
}

/// Reconstruct primes <= sqrt(stop) from 1 bits of the
/// sieve array and add them to PrimeGen
///
void SievingPrimes::generatePrimes(const byte_t* sieve, uint_t sieveSize)
{
  uint64_t low = getSegmentLow();

  for (uint_t i = 0; i < sieveSize; i += 8)
  {
    uint64_t bits = littleendian_cast<uint64_t>(&sieve[i]);
    while (bits)
      primeGen_.addSievingPrime((uint_t) nextPrime(&bits, low));

    low += NUMBERS_PER_BYTE * 8;
  }
}

} // namespace
