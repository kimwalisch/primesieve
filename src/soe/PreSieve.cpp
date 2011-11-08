//
// Copyright (c) 2011 Kim Walisch, <kim.walisch@gmail.com>.
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
#include "SieveOfEratosthenes.h"
#include "defs.h"

#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <cassert>

const uint32_t PreSieve::smallPrimes_[10] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 31 };

const uint32_t PreSieve::unsetBits_[30] = {
    BIT0, 0xFF, 0xFF, 0xFF, BIT1, 0xFF, BIT2, 0xFF, 0xFF, 0xFF,
    BIT3, 0xFF, BIT4, 0xFF, 0xFF, 0xFF, BIT5, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, BIT6, 0xFF, BIT7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

/**
 * Pre-sieve multiples of small primes <= limit to speed up
 * the sieve of Eratosthenes.
 * @pre limit >= 13 && <= 23
 * @see PreSieve.h for more information.
 */
PreSieve::PreSieve(uint32_t limit) :
  limit_(limit),
  wheelArray_(NULL),
  size_(0)
{
  // limit_ <= 23 prevents 32 bit overflows
  if (limit_ < 13 || limit_ > 23)
    throw std::overflow_error("PreSieve: limit must be >= 13 && <= 23.");
  primeProduct_ = this->getPrimeProduct(limit_);
  size_ = primeProduct_ / SieveOfEratosthenes::NUMBERS_PER_BYTE;
  this->initWheelArray();
}

PreSieve::~PreSieve() {
  delete[] wheelArray_;
}

uint32_t PreSieve::getPrimeProduct(uint32_t limit) const {
  assert(limit < smallPrimes_[9]);
  uint32_t pp = 1;
  for (int i = 0; smallPrimes_[i] <= limit; i++)
    pp *= smallPrimes_[i];
  return pp;
}

/**
 * Allocate the wheelArray_ and remove (unset bits) the multiples of
 * small primes <= limit_ from it.
 */
void PreSieve::initWheelArray() {
  static_assert(SieveOfEratosthenes::NUMBERS_PER_BYTE == 30, 
               "SieveOfEratosthenes::NUMBERS_PER_BYTE == 30");
  assert(limit_ < smallPrimes_[9]);
  assert(size_ > 0);
  wheelArray_ = new uint8_t[size_];
  wheelArray_[0] = 0xFF;
  uint32_t primeProduct = 2 * 3 * 5;

  for (int i = 3; smallPrimes_[i] <= limit_; i++) {
    // cross-off the multiples of primes < smallPrimes_[i]
    // up to the current primeProduct
    uint32_t pp30 = primeProduct / SieveOfEratosthenes::NUMBERS_PER_BYTE;
    for (uint32_t j = 1; j < smallPrimes_[i]; j++) {
      assert((j + 1) * pp30 <= size_);
      std::memcpy(&wheelArray_[j * pp30], wheelArray_, pp30);
    }
    // next prime product
    primeProduct *= smallPrimes_[i];
    // '- 7' is a correction for primes of type i*30 + 31
    uint32_t multiple = smallPrimes_[i] - 7;
    // cross-off the multiples (unset bits) of smallPrimes_[i]
    // up to the current primeProduct
    for (; multiple < primeProduct; multiple += smallPrimes_[i] * 2) {
      uint32_t multipleIndex = multiple / SieveOfEratosthenes::NUMBERS_PER_BYTE;
      uint32_t bitPosition   = multiple % SieveOfEratosthenes::NUMBERS_PER_BYTE;
      assert(multipleIndex < size_);
      wheelArray_[multipleIndex] &= unsetBits_[bitPosition];
    }
  }
}

/**
 * Pre-sieve multiples of small primes <= getLimit() (e.g. 19) to
 * speed up the sieve of Eratosthenes.
 * @see PreSieve.h for more information.
 */
void PreSieve::doIt(uint8_t* sieve, 
                    uint32_t sieveSize,
                    uint64_t segmentLow) const
{
  // map segmentLow to the wheelArray_
  uint32_t offset = static_cast<uint32_t> (segmentLow % primeProduct_) / 
      SieveOfEratosthenes::NUMBERS_PER_BYTE;
  uint32_t sizeLeft = size_ - offset;

  if (sizeLeft > sieveSize) {
    // copy a chunk of sieveSize bytes to sieve
    std::memcpy(sieve, &wheelArray_[offset], sieveSize);
  } else {
    // copy the last remaining bytes at the end of wheelArray_
    // to the beginning of the sieve array
    std::memcpy(sieve, &wheelArray_[offset], sizeLeft);
    offset = sizeLeft;
    // restart copying at the beginning of wheelArray_
    for (; offset + size_ < sieveSize; offset += size_) {
      std::memcpy(&sieve[offset], wheelArray_, size_);
    }
    std::memcpy(&sieve[offset], wheelArray_, sieveSize - offset);
  }
}
