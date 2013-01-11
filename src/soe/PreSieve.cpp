///
/// @file   PreSieve.cpp
/// @brief  Pre-sieve multiples of small primes to speed up the
///         sieve of Eratosthenes.
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the New BSD License. See the
/// LICENSE file in the top level directory.
///

#include "config.h"
#include "PreSieve.h"
#include "EratSmall.h"
#include "imath.h"

#include <stdint.h>
#include <cstring>

namespace soe {

const uint_t PreSieve::primes_[10] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29 };

/// PreSieve multiples of small primes <= limit.
/// @pre limit >= 13 && <= 23
///
PreSieve::PreSieve(int limit)
{
  // limit_ <= 23 prevents 32-bit overflows
  limit_ = getInBetween(13, limit, 23);
  primeProduct_ = 1;
  for (int i = 0; primes_[i] <= limit_; i++)
    primeProduct_ *= primes_[i];
  size_ = primeProduct_ / 30;
  preSieved_ = new byte_t[size_];
  init();
}

PreSieve::~PreSieve()
{
  delete[] preSieved_;
}

/// Cross-off the multiples of small primes <= limit_
/// in the preSieved_ array.
///
void PreSieve::init()
{
  std::memset(preSieved_, 0xff, size_);
  uint_t start = primeProduct_;
  uint_t stop  = primeProduct_ * 2;
  EratSmall eratSmall(stop, size_, limit_);
  for (int i = 3; primes_[i] <= limit_; i++)
    eratSmall.add(primes_[i], start);
  eratSmall.crossOff(preSieved_, &preSieved_[size_]);
}

/// Pre-sieve the multiples of small primes <= limit_
/// in the sieve array.
///
void PreSieve::doIt(byte_t* sieve, uint_t sieveSize, uint64_t segmentLow) const
{
  // map segmentLow to the preSieved_ array
  uint_t remainder = static_cast<uint_t>(segmentLow % primeProduct_);
  uint_t offset = remainder / 30;
  uint_t sizeLeft = size_ - offset;

  if (sieveSize <= sizeLeft)
    std::memcpy(sieve, &preSieved_[offset], sieveSize);
  else {
    // copy the last remaining bytes at the end of preSieved_
    // to the beginning of the sieve array
    std::memcpy(sieve, &preSieved_[offset], sizeLeft);
    // restart copying at the beginning of preSieved_
    for (offset = sizeLeft; offset + size_ < sieveSize; offset += size_)
      std::memcpy(&sieve[offset], preSieved_, size_);
    std::memcpy(&sieve[offset], preSieved_, sieveSize - offset);
  }
}

} // namespace soe
