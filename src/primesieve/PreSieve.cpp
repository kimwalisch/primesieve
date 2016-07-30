///
/// @file   PreSieve.cpp
/// @brief  Pre-sieve multiples of small primes to speed up the
///         sieve of Eratosthenes.
///
/// Copyright (C) 2016 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/config.hpp>
#include <primesieve/PreSieve.hpp>
#include <primesieve/EratSmall.hpp>

#include <stdint.h>
#include <algorithm>
#include <cstring>

using namespace std;
using namespace primesieve;

namespace {

const uint_t primes[] = { 2, 3, 5, 7, 11, 13, 17, 19, 23 };

const uint_t primeProduct[] = { 2, 6, 30, 210, 2310, 30030, 510510, 9699690, 223092870 };

}

namespace primesieve {

PreSieve::PreSieve(uint64_t start, uint64_t stop) :
  limit_(7),
  preSieve_(NULL)
{
  // only use a large preSieve_ buffer if the
  // sieve interval is sufficiently large
  uint64_t distance = stop - start;
  uint64_t threshold = max(isqrt(stop), distance) / 10;

  for (int i = 4; i < 8; i++)
    if (threshold > primeProduct[i])
      limit_ = primes[i];

  init();
}

PreSieve::~PreSieve()
{
  delete[] preSieve_;
}

/// Cross-off the multiples of small primes <= limit_
/// in the preSieve_ array.
///
void PreSieve::init()
{
  for (int i = 0; primes[i] <= limit_; i++)
    primeProduct_ = primeProduct[i];

  size_ = primeProduct_ / NUMBERS_PER_BYTE;
  preSieve_ = new byte_t[size_];
  memset(preSieve_, 0xff, size_);

  uint_t stop = primeProduct_ * 2;
  EratSmall eratSmall(stop, size_, limit_);

  for (int i = 3; primes[i] <= limit_; i++)
    eratSmall.addSievingPrime(primes[i], primeProduct_);

  // sieve [primeProduct_, primeProduct_ * 2]
  eratSmall.crossOff(preSieve_, &preSieve_[size_]);
}

/// Pre-sieve the multiples of small primes <= limit_
/// by copying preSieve_ to sieve.
///
void PreSieve::doIt(byte_t* sieve,
                    uint_t sieveSize,
                    uint64_t segmentLow) const
{
  // map segmentLow to the preSieve_ array
  uint_t remainder = static_cast<uint_t>(segmentLow % primeProduct_);
  uint_t index = remainder / NUMBERS_PER_BYTE;
  uint_t sizeLeft = size_ - index;

  if (sieveSize <= sizeLeft)
    memcpy(sieve, &preSieve_[index], sieveSize);
  else
  {
    // copy the last remaining bytes at the end of preSieve_
    // to the beginning of the sieve array
    memcpy(sieve, &preSieve_[index], sizeLeft);

    // restart copying at the beginning of preSieve_
    for (index = sizeLeft; index + size_ < sieveSize; index += size_)
      memcpy(&sieve[index], preSieve_, size_);

    memcpy(&sieve[index], preSieve_, sieveSize - index);
  }
}

} // namespace
