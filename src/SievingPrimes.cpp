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
#include <primesieve/Erat.hpp>
#include <primesieve/PreSieve.hpp>
#include <primesieve/littleendian_cast.hpp>
#include <primesieve/pmath.hpp>

#include <stdint.h>
#include <vector>

namespace primesieve {

SievingPrimes::SievingPrimes(Erat* erat, PreSieve& preSieve)
{
  init(erat, preSieve);
}

void SievingPrimes::init(Erat* erat, PreSieve& preSieve)
{
  Erat::init(preSieve.getMaxPrime() + 1,
             isqrt(erat->getStop()),
             erat->getSieveSize(),
             preSieve);

  tinySieve();
}

/// Sieve up to n^(1/4)
void SievingPrimes::tinySieve()
{
  uint64_t n = isqrt(stop_);
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
  if (sieveIdx_ >= sieveSize_)
    if (!sieveSegment())
      return;

  uint64_t num = 0;
  uint64_t bits = littleendian_cast<uint64_t>(&sieve_[sieveIdx_]);
  sieveIdx_ += 8;

  for (; bits != 0; num++)
    primes_[num] = nextPrime(&bits, low_);

  i_ = 0;
  size_ = num;
  low_ += 8 * 30;
}

bool SievingPrimes::sieveSegment()
{
  if (hasNextSegment())
  {
    sieveIdx_ = 0;
    uint64_t high = segmentHigh_;

    for (uint64_t& i = tinyIdx_; i * i <= high; i += 2)
      if (tinySieve_[i])
        addSievingPrime(i);

    Erat::sieveSegment();
    return true;
  }
  else
  {
    i_ = 0;
    size_ = 1;
    primes_[0] = ~0ull;
    return false;
  }
}

} // namespace
