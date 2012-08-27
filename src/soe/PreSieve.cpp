//
// Copyright (c) 2012 Kim Walisch, <kim.walisch@gmail.com>.
// All rights reserved.
//
// This file is part of primesieve.
// Homepage: http://primesieve.googlecode.com
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//   * Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above
//     copyright notice, this list of conditions and the following
//     disclaimer in the documentation and/or other materials provided
//     with the distribution.
//   * Neither the name of the author nor the names of its
//     contributors may be used to endorse or promote products derived
//     from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.

#include "PreSieve.h"
#include "EratSmall.h"
#include "imath.h"
#include "config.h"

#include <stdint.h>
#include <cstring>

namespace soe {

const uint_t PreSieve::primes_[10] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29 };

/// Create a new PreSieve object to pre-sieve multiples
/// of small primes <= limit in SieveOfEratosthenes.
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
  preSieved_ = new uint8_t[size_];
  init();
}

PreSieve::~PreSieve()
{
  delete[] preSieved_;
}

/// Cross-off the multiples of small primes <= limit_
/// from the preSieved_ array.
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

/// Pre-sieve multiples of small primes <= getLimit()
/// to speed up SieveOfEratosthenes.
///
void PreSieve::doIt(uint8_t* sieve, uint_t sieveSize, uint64_t segmentLow) const
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
