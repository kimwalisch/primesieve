///
/// @file  SievingPrimes.cpp
///        Generates the sieving primes up n^(1/2).
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
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
#include <cassert>
#include <vector>

namespace primesieve {

SievingPrimes::SievingPrimes(Erat* erat,
                             PreSieve& preSieve,
                             MemoryPool& memoryPool)
{
  init(erat, preSieve, memoryPool);
}

void SievingPrimes::init(Erat* erat,
                         PreSieve& preSieve,
                         MemoryPool& memoryPool)
{
  assert(preSieve.getMaxPrime() >= 7);
  uint64_t start = preSieve.getMaxPrime() + 2;
  uint64_t stop = isqrt(erat->getStop());
  uint64_t sieveSize = erat->getSieveSize();
  Erat::init(start, stop, sieveSize, preSieve, memoryPool);

  assert(start % 2 == 1);
  tinyIdx_ = start;
  low_ = segmentLow_;

  if (start * start <= stop)
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
}

void SievingPrimes::fill()
{
  if (sieveIdx_ >= sieveSize_)
    if (!sieveSegment())
      return;

  size_t num = 0;
  uint64_t low = low_;
  assert(primes_.size() >= 64);

  // Fill the buffer with at least (primes_.size() - 64) primes.
  // Each loop iteration can generate up to 64 primes
  // so we have to stop generating primes once there is
  // not enough space for 64 more primes.
  do
  {
      uint64_t bits = littleendian_cast<uint64_t>(&sieve_[sieveIdx_]);
      size_t j = num;
      num += popcnt64(bits);

      do
      {
        assert(j + 4 < primes_.size());
        primes_[j+0] = nextPrime(bits, low); bits &= bits - 1;
        primes_[j+1] = nextPrime(bits, low); bits &= bits - 1;
        primes_[j+2] = nextPrime(bits, low); bits &= bits - 1;
        primes_[j+3] = nextPrime(bits, low); bits &= bits - 1;
        j += 4;
      }
      while (j < num);

      low += 8 * 30;
      sieveIdx_ += 8;
  }
  while (num <= primes_.size() - 64 &&
         sieveIdx_ < sieveSize_);

  low_ = low;
  i_ = 0;
  size_ = num;
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
