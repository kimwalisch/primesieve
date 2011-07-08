//
// Copyright (c) 2011 Kim Walisch, <kim.walisch@gmail.com>.
// All rights reserved.
//
// This file is part of primesieve.
// Visit: http://primesieve.googlecode.com
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
//   * Neither the name of the modp.com nor the names of its
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
#include "SieveOfEratosthenes.h"

#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <cassert>

const uint32_t PreSieve::smallPrimes_[9] = { 2, 3, 5, 7, 11, 13, 17, 19, 23 };

/**
 * Pre-sieve multiples of small primes <= limit to speed up the sieve
 * of Eratosthenes.
 * @pre limit >= 11 && limit <= 23
 * @see SieveOfEratosthenes::sieve(uint32_t)
 */
PreSieve::PreSieve(uint32_t limit) : wheelArray_(NULL), size_(0) {
  // limit <= 23 prevents 32 bit overflows
  if (limit < 11 || limit > 23)
    throw std::overflow_error("PreSieve: limit must be >= 11 && <= 23.");
  limit_ = limit;
  primeProduct_ = this->getPrimeProduct(limit_);
  size_ = primeProduct_ / SieveOfEratosthenes::NUMBERS_PER_BYTE;
  this->initWheelArray();
}

PreSieve::~PreSieve() {
  delete[] wheelArray_;
}

uint32_t PreSieve::getPrimeProduct(uint32_t limit) const {
  assert(limit <= 23);
  uint32_t pp = 1;
  for (int i = 0; i < 9 && smallPrimes_[i] <= limit; i++)
    pp *= smallPrimes_[i];
  return pp;
}

/**
 * Allocate the wheelArray_ and remove the multiples of small
 * primes <= limit_ from it.
 */
void PreSieve::initWheelArray() {
  assert(SieveOfEratosthenes::NUMBERS_PER_BYTE == 30);
  assert(size_ > 0);
  const uint32_t unsetBit[30] = {
      BIT0, 0xff, 0xff, 0xff, BIT1, 0xff,
      BIT2, 0xff, 0xff, 0xff, BIT3, 0xff,
      BIT4, 0xff, 0xff, 0xff, BIT5, 0xff,
      0xff, 0xff, 0xff, 0xff, BIT6, 0xff,
      BIT7, 0xff, 0xff, 0xff, 0xff, 0xff };

  wheelArray_ = new uint8_t[size_];
  // initialization, set bits of the first byte to 1
  wheelArray_[0] = 0xff;
  uint32_t primeProduct = 2 * 3 * 5;

  for (int i = 3; i < 9 && smallPrimes_[i] <= limit_; i++) {
    // cross off the multiples of primes < smallPrimes_[i]
    // up to the current prime product
    uint32_t pp30 = primeProduct / SieveOfEratosthenes::NUMBERS_PER_BYTE;
    for (uint32_t j = 1; j < smallPrimes_[i]; j++) {
      assert((j + 1) * pp30 <= size_);
      std::memcpy(&wheelArray_[j * pp30], wheelArray_, pp30);
    }
    primeProduct *= smallPrimes_[i];
    // '- 7' is a correction for primes of type i*30 + 31
    uint32_t multiple = smallPrimes_[i] - 7;
    // cross off the multiples of smallPrimes_[i] up to the current
    // prime product
    while (multiple < primeProduct) {
      uint32_t multipleIndex = multiple / SieveOfEratosthenes::NUMBERS_PER_BYTE;
      uint32_t bitPosition = multiple % SieveOfEratosthenes::NUMBERS_PER_BYTE;
      assert(multipleIndex < size_);
      wheelArray_[multipleIndex] &= unsetBit[bitPosition];
      multiple += smallPrimes_[i] * 2;
    }
  }
}

/**
 * Pre-sieve multiples of small primes <= limit_ (e.g. 19).
 * Resets the sieve array (resets bits to 1) of SieveOfEratosthenes
 * objects after each sieved segment and removes the multiples of
 * small primes without sieving.
 * @see SieveOfEratosthenes::sieve(uint32_t)
 */
void PreSieve::doIt(uint8_t* sieve, 
                    uint32_t sieveSize,
                    uint64_t segmentLow) const
{
  // calculate the position of segmentLow within the wheelArray_
  uint32_t sieveOffset = static_cast<uint32_t> (
      segmentLow % primeProduct_) / SieveOfEratosthenes::NUMBERS_PER_BYTE;
  uint32_t sizeLeft = size_ - sieveOffset;

  if (sizeLeft > sieveSize) {
    // copy a chunk of sieveSize bytes to the sieve array
    std::memcpy(sieve, &wheelArray_[sieveOffset], sieveSize);
  } else {
    // copy the last remaining bytes at the end of wheelArray_ to the
    // beginning of the sieve array
    std::memcpy(sieve, &wheelArray_[sieveOffset], sizeLeft);
    // restart copying at the beginning of wheelArray_
    sieveOffset = sizeLeft;
    while (sieveOffset + size_ < sieveSize) {
      std::memcpy(&sieve[sieveOffset], wheelArray_, size_);
      sieveOffset += size_;
    }
    std::memcpy(&sieve[sieveOffset], wheelArray_, sieveSize - sieveOffset);
  }
}
