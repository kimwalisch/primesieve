///
/// @file  NextPrime.cpp
///        This class implements a nextPrime() method
///        for iterating over primes.
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/NextPrime.hpp>
#include <primesieve/PreSieve.hpp>
#include <primesieve/Erat.hpp>
#include <primesieve/littleendian_cast.hpp>
#include <primesieve/SievingPrimes.hpp>

#include <stdint.h>
#include <algorithm>

using namespace std;

namespace primesieve {

NextPrime::NextPrime(uint64_t start,
                     uint64_t stop,
                     uint64_t sieveSize)
  : preSieve_(start, stop)
{
  start = max<uint64_t>(7, start);
  Erat::init(start, stop, sieveSize, preSieve_);
  sievingPrimes_.init(this, preSieve_);
}

void NextPrime::fill()
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

bool NextPrime::sieveSegment()
{
  if (hasNextSegment())
  {
    sieveIdx_ = 0;
    uint64_t high = min(segmentHigh_, stop_);
    uint64_t sqrtHigh = isqrt(high);

    if (!sievingPrime_)
      sievingPrime_ = sievingPrimes_.nextPrime();

    while (sievingPrime_ <= sqrtHigh)
    {
      addSievingPrime(sievingPrime_);
      sievingPrime_ = sievingPrimes_.nextPrime();
    }

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
