///
/// @file   PreSieve.cpp
/// @brief  Pre-sieve multiples of small primes to speed up the
///         sieve of Eratosthenes.
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
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
#include <cstring>
#include <memory>

using namespace std;
using namespace primesieve;

namespace {

const uint_t primes[] = { 2, 3, 5, 7, 11, 13, 17, 19, 23 };

const uint_t primeProduct[] = { 2, 6, 30, 210, 2310, 30030, 510510, 9699690, 223092870 };

} // namespace

namespace primesieve {

PreSieve::PreSieve(uint64_t start, uint64_t stop) :
  limit_(7),
  buffer_(nullptr)
{
  // use a small buffer_ array if the
  // sieve interval is small
  uint64_t distance = stop - start;
  uint64_t threshold = max(isqrt(stop), distance) / 10;

  for (int i = 4; i < 8; i++)
    if (threshold > primeProduct[i])
      limit_ = primes[i];

  for (int i = 0; primes[i] <= limit_; i++)
    primeProduct_ = primeProduct[i];

  allocate();
}

void PreSieve::allocate()
{
  size_ = primeProduct_ / NUMBERS_PER_BYTE;
  deleter_.reset(new byte_t[size_]);
  buffer_ = deleter_.get();
  memset(buffer_, 0xff, size_);

  uint_t stop = primeProduct_ * 2;
  EratSmall eratSmall(stop, size_, limit_);

  for (int i = 3; primes[i] <= limit_; i++)
    eratSmall.addSievingPrime(primes[i], primeProduct_);

  // cross-off the multiples of small
  // primes <= limit in buffer
  eratSmall.crossOff(buffer_, &buffer_[size_]);
}

/// Pre-sieve multiples of small primes <= limit
/// by copying buffer_ to sieve
///
void PreSieve::copy(byte_t* sieve,
                    uint_t sieveSize,
                    uint64_t segmentLow) const
{
  // map segmentLow to the buffer array
  uint_t remainder = (uint_t)(segmentLow % primeProduct_);
  uint_t index = remainder / NUMBERS_PER_BYTE;
  uint_t sizeLeft = size_ - index;

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
