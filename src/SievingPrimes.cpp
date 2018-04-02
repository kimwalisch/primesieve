///
/// @file  SievingPrimes.cpp
///        Generates the sieving primes up n^(1/2).
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/SievingPrimes.hpp>
#include <primesieve/PrimeGenerator.hpp>
#include <primesieve/PreSieve.hpp>
#include <primesieve/Erat.hpp>
#include <primesieve/littleendian_cast.hpp>

#include <stdint.h>
#include <algorithm>
#include <vector>

using namespace std;

namespace primesieve {

SievingPrimes::SievingPrimes(PrimeGenerator& primeGen, const PreSieve& preSieve)
{
  Erat::init(preSieve.getMaxPrime() + 1,
             primeGen.getSqrtStop(),
             primeGen.getSieveSize(),
             preSieve);
}

/// Sieve up to n^(1/4)
void SievingPrimes::tinySieve()
{
  uint64_t n = sqrtStop_;
  tinySieve_.resize(n + 1, true);

  for (uint64_t i = 3; i * i <= n; i += 2)
    if (tinySieve_[i])
      for (uint64_t j = i * i; j <= n; j += i * 2)
        tinySieve_[j] = false;

  tinyIdx_ = start_;
  tinyIdx_ += ~tinyIdx_ & 1;
}

void SievingPrimes::fill()
{
  i_ = 0;

  if (sieveIdx_ >= sieveSize_)
    if (!sieveSegment())
      return;

  uint64_t bits = littleendian_cast<uint64_t>(&sieve_[sieveIdx_]);
  sieveIdx_ += 8;
  uint64_t num = 0;

  for (; bits != 0; num++)
    primes_[num] = getPrime(&bits, low_);

  num_ = num;
  low_ += 30 * 8;
}

bool SievingPrimes::sieveSegment()
{
  if (hasNextSegment())
  {
    sieveIdx_ = 0;
    uint64_t high = segmentHigh_;
    uint64_t max = std::min(high, stop_);

    if (tinySieve_.empty())
      tinySieve();

    for (uint64_t& i = tinyIdx_; i * i <= max; i += 2)
      if (tinySieve_[i])
        addSievingPrime(i);

    Erat::sieveSegment();
    return true;
  }
  else
  {
    num_ = 1;
    primes_[0] = ~0ull;
    return false;
  }
}

} // namespace
