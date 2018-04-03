///
/// @file   PreSieve.cpp
/// @brief  Pre-sieve multiples of small primes to speed up
///         the sieve of Eratosthenes.
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/config.hpp>
#include <primesieve/PreSieve.hpp>
#include <primesieve/EratSmall.hpp>
#include <primesieve/pmath.hpp>

#include <stdint.h>
#include <algorithm>
#include <array>
#include <cstring>
#include <memory>

using namespace std;
using namespace primesieve;

namespace {

// small primes >= 7
const array<uint64_t, 5> primes = { 7, 11, 13, 17, 19 };

// prime products of primes >= 7
const array<uint64_t, 5> primeProducts = { 210, 2310, 30030, 510510, 9699690 };

} // namespace

namespace primesieve {

PreSieve::PreSieve(uint64_t start, uint64_t stop)
{
  // use a small buffer_ array if the
  // sieve interval is small
  uint64_t distance = stop - start;
  uint64_t threshold = max(distance, isqrt(stop)) / 10;
  size_t i = 0;

  for (uint64_t pp : primeProducts)
    if (pp < threshold)
      i += 1;

  i = min(i, primes.size() - 1);
  maxPrime_ = primes[i];
  primeProduct_ = primeProducts[i];
}

/// Pre-sieve a small buffer by removing the
/// multiples of primes <= maxPrime.
///
void PreSieve::init()
{
  size_ = primeProduct_ / 30;
  deleter_.reset(new byte_t[size_]);
  buffer_ = deleter_.get();
  memset(buffer_, 0xff, size_);

  EratSmall eratSmall;
  uint64_t stop = primeProduct_ * 2;
  eratSmall.init(stop, size_, maxPrime_);

  for (uint64_t prime : primes)
    if (prime <= maxPrime_)
      eratSmall.addSievingPrime(prime, primeProduct_);

  eratSmall.crossOff(buffer_, size_);
}

/// Copy pre-sieved buffer to sieve array
void PreSieve::copy(byte_t* sieve,
                    uint64_t sieveSize,
                    uint64_t segmentLow)
{
  if (!buffer_)
    init();

  // find segmentLow index
  uint64_t remainder = segmentLow % primeProduct_;
  uint64_t index = remainder / 30;
  uint64_t sizeLeft = size_ - index;

  if (sieveSize <= sizeLeft)
    memcpy(sieve, &buffer_[index], sieveSize);
  else
  {
    // copy the last remaining bytes at the end of buffer
    // to the beginning of the sieve array
    memcpy(sieve, &buffer_[index], sizeLeft);

    // restart copying at the beginning of buffer
    for (index = sizeLeft; index + size_ < sieveSize; index += size_)
      memcpy(&sieve[index], buffer_, size_);

    memcpy(&sieve[index], buffer_, sieveSize - index);
  }
}

} // namespace
