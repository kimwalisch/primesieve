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

#include <primesieve/PreSieve.hpp>
#include <primesieve/EratSmall.hpp>
#include <primesieve/pmath.hpp>
#include <primesieve/types.hpp>

#include <stdint.h>
#include <algorithm>
#include <array>
#include <iterator>
#include <memory>

using namespace std;

namespace {

// small primes >= 7
const array<uint64_t, 5> primes = { 7, 11, 13, 17, 19 };

// prime products of primes >= 7
const array<uint64_t, 5> primeProducts = { 210, 2310, 30030, 510510, 9699690 };

} // namespace

namespace primesieve {

void PreSieve::init(uint64_t start,
                    uint64_t stop)
{
  // the pre-sieve buffer should be at least 10
  // times smaller than the sieving distance
  // in order to reduce initialization overhead
  uint64_t dist = stop - start;
  uint64_t threshold = max(dist, isqrt(stop)) / 100;
  auto last =  primeProducts.end() - 1;
  auto iter = lower_bound(primeProducts.begin(), last, threshold);
  auto i = distance(primeProducts.begin(), iter);

  if (primes.at(i) > maxPrime_)
    initBuffer(primes[i], primeProducts[i]);
}

/// Initialize the buffer by removing the
/// multiples of primes <= maxPrime.
///
void PreSieve::initBuffer(uint64_t maxPrime,
                          uint64_t primeProduct)
{
  maxPrime_ = maxPrime;
  primeProduct_ = primeProduct;
  size_ = primeProduct_ / 30;

  buffer_ = new byte_t[size_];
  deleter_.reset(buffer_);
  fill_n(buffer_, size_, 0xff);

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
                    uint64_t segmentLow) const
{
  // find segmentLow index
  uint64_t remainder = segmentLow % primeProduct_;
  uint64_t i = remainder / 30;
  uint64_t sizeLeft = size_ - i;

  if (sieveSize <= sizeLeft)
    copy_n(&buffer_[i], sieveSize, sieve);
  else
  {
    // copy the last remaining bytes of buffer
    // to the beginning of the sieve array
    copy_n(&buffer_[i], sizeLeft, sieve);

    // restart copying at the beginning of buffer
    for (i = sizeLeft; i + size_ < sieveSize; i += size_)
      copy_n(buffer_, size_, &sieve[i]);

    // copy the last remaining bytes
    copy_n(buffer_, sieveSize - i, &sieve[i]);
  }
}

} // namespace
