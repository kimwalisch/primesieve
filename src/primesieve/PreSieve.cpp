///
/// @file   PreSieve.cpp
/// @brief  Pre-sieve multiples of small primes to speed up the
///         sieve of Eratosthenes.
///
/// Copyright (C) 2015 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/config.hpp>
#include <primesieve/PreSieve.hpp>
#include <primesieve/EratSmall.hpp>
#include <primesieve/primesieve_error.hpp>

#include <stdint.h>
#include <cstring>

using std::memcpy;
using std::memset;

namespace primesieve {

const uint_t PreSieve::primes_[10] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29 };

/// PreSieve multiples of small primes <= limit.
/// @pre limit >= 11 && <= 23
///
PreSieve::PreSieve(int limit) :
  limit_(limit),
  preSieved_(NULL)
{
  // limit_ <= 23 prevents 32-bit overflows
  if (limit_ < 11 || limit_ > 23)
    throw primesieve_error("PreSieve: limit must be >= 11 && <= 23");
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
  primeProduct_ = 1;
  for (int i = 0; primes_[i] <= limit_; i++)
    primeProduct_ *= primes_[i];

  size_ = primeProduct_ / NUMBERS_PER_BYTE;
  preSieved_ = new byte_t[size_];
  memset(preSieved_, 0xff, size_);

  uint_t stop = primeProduct_ * 2;
  EratSmall eratSmall(stop, size_, limit_);

  for (int i = 3; primes_[i] <= limit_; i++)
    eratSmall.addSievingPrime(primes_[i], primeProduct_);

  // sieve [primeProduct_, primeProduct_ * 2]
  eratSmall.crossOff(preSieved_, &preSieved_[size_]);
}

/// Pre-sieve the multiples of small primes <= limit_
/// in the sieve array.
///
void PreSieve::doIt(byte_t* sieve, uint_t sieveSize, uint64_t segmentLow) const
{
  // map segmentLow to the preSieved_ array
  uint_t remainder = static_cast<uint_t>(segmentLow % primeProduct_);
  uint_t index = remainder / NUMBERS_PER_BYTE;
  uint_t sizeLeft = size_ - index;

  if (sieveSize <= sizeLeft)
    memcpy(sieve, &preSieved_[index], sieveSize);
  else {
    // copy the last remaining bytes at the end of preSieved_
    // to the beginning of the sieve array
    memcpy(sieve, &preSieved_[index], sizeLeft);

    // restart copying at the beginning of preSieved_
    for (index = sizeLeft; index + size_ < sieveSize; index += size_)
      memcpy(&sieve[index], preSieved_, size_);

    memcpy(&sieve[index], preSieved_, sieveSize - index);
  }
}

} // namespace primesieve
