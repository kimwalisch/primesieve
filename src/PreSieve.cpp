///
/// @file   PreSieve.cpp
/// @brief  Pre-sieve multiples of small primes to speed up the sieve
///         of Eratosthenes. At startup primesieve initializes a small
///         buffer of size p1*p2*p3*pn and removes the multiples of
///         the first n primes from that buffer. Then while sieving at
///         the start of each new segment this buffer is simply copied
///         to the sieve array and now we can start sieving at p(n+1)
///         instead of p1. By default primesieve pre-sieves multiples
///         of primes <= 19, in practice pre-sieving using even larger
///         primes uses too much memory and slows things down. In
///         primesieve pre-sieving provides a minor speed of up to 20%
///         when the sieving distance is relatively small
///         e.g. < 10^10.
///
///         The pre-sieve buffer can be both smaller or larger than
///         the actual sieve array so a little care needs to be taken
///         when copying the buffer to the sieve array. When the
///         buffer is smaller than the sieve array we need to
///         repeatedly copy the buffer to the sieve array until the
///         sieve array has been filled completely. When the buffer is
///         larger than the sieve array we only need to partially copy
///         it to the sieve array.
///
/// Copyright (C) 2019 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/PreSieve.hpp>
#include <primesieve/EratSmall.hpp>
#include <primesieve/pmath.hpp>

#include <stdint.h>
#include <algorithm>
#include <array>
#include <iterator>
#include <memory>

using namespace std;

namespace {

// Small primes >= 7
const array<uint64_t, 5> primes = { 7, 11, 13, 17, 19 };

// Prime products of primes >= 7
const array<uint64_t, 5> primeProducts = { 210, 2310, 30030, 510510, 9699690 };

} // namespace

namespace primesieve {

void PreSieve::init(uint64_t start,
                    uint64_t stop)
{
  // The pre-sieve buffer should be at least 100
  // times smaller than the sieving distance
  // in order to reduce initialization overhead.
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

  buffer_ = new uint8_t[size_];
  deleter_.reset(buffer_);
  fill_n(buffer_, size_, (uint8_t) 0xff);

  EratSmall eratSmall;
  uint64_t stop = primeProduct_ * 2;
  eratSmall.init(stop, size_, maxPrime_);

  for (uint64_t prime : primes)
    if (prime <= maxPrime_)
      eratSmall.addSievingPrime(prime, primeProduct_);

  eratSmall.crossOff(buffer_, size_);
}

/// Copy pre-sieved buffer to sieve array
void PreSieve::copy(uint8_t* sieve,
                    uint64_t sieveSize,
                    uint64_t segmentLow) const
{
  // Find segmentLow index
  uint64_t remainder = segmentLow % primeProduct_;
  uint64_t i = remainder / 30;
  uint64_t sizeLeft = size_ - i;

  if (sieveSize <= sizeLeft)
    copy_n(&buffer_[i], sieveSize, sieve);
  else
  {
    // Copy the last remaining bytes of buffer
    // to the beginning of the sieve array
    copy_n(&buffer_[i], sizeLeft, sieve);

    // Restart copying at the beginning of buffer
    for (i = sizeLeft; i + size_ < sieveSize; i += size_)
      copy_n(buffer_, size_, &sieve[i]);

    // Copy the last remaining bytes
    copy_n(buffer_, sieveSize - i, &sieve[i]);
  }
}

} // namespace
