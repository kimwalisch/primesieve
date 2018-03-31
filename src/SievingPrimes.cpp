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
  SieveOfEratosthenes(preSieve.getMaxPrime() + 1,
                      primeGen.getSqrtStop(),
                      primeGen.getSieveSize() / 1024,
                      preSieve),
  primeGen_(primeGen)
{
  low_ = getSegmentLow();
  sieveIdx_ = 0;
  num_ = 0;

  tinyPrimes();
}

uint64_t SievingPrimes::next_prime()
{
  while (!num_)
    fill();

  return primes_[--num_];
}

void SievingPrimes::fill()
{
  if (sieveIdx_ >= sieveSize_)
    sieveOneSegment();

  uint64_t bits = littleendian_cast<uint64_t>(&sieve_[sieveIdx_]);

  while (bits)
    primes_[num_++] = nextPrime(&bits, low_);

  low_ += NUMBERS_PER_BYTE * 8;
  sieveIdx_++;
}

void SievingPrimes::tinyPrimes()
{
  // we need to sieve up to the first prime > sqrt(stop)
  uint64_t n = getSqrtStop();
  uint64_t maxPrimeGap = max_prime_gap(n + 10);
  n += maxPrimeGap;

  isPrime_.resize(n + 1, true);

  for (uint64_t i = 3; i * i <= n; i += 2)
    if (isPrime_[i])
      for (uint64_t j = i * i; j <= n; j += i * 2)
        isPrime_[j] = false;

  tinyIdx_ = getStart();
  tinyIdx_ += ~tinyIdx_ & 1;
}

void SievingPrimes::sieveOneSegment()
{
  sieveIdx_ = 0;
  uint64_t high = getSegmentHigh();
  uint64_t max = isPrime_.size() - 1;
  high = std::min(high, max);

  for (uint64_t& i = tinyIdx_; i * i <= high; i += 2)
    if (isPrime_[i])
      addSievingPrime(i);

  sieveSegment();
}

} // namespace
