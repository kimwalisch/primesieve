/*
 * PreSieve.cpp -- This file is part of primesieve
 *
 * Copyright (C) 2011 Kim Walisch, <kim.walisch@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include "PreSieve.h"
#include "SieveOfEratosthenes.h"
#include "pmath.h"

#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <cassert>

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
  primeProduct_ = primeProduct(limit_);
  size_ = primeProduct_ / SieveOfEratosthenes::NUMBERS_PER_BYTE;
  this->initWheelArray();
}

PreSieve::~PreSieve() {
  delete[] wheelArray_;
}

/**
 * Allocate the wheelArray_ and remove the multiples of small
 * primes <= limit_ from it.
 */
void PreSieve::initWheelArray() {
  assert(size_ > 0);
  wheelArray_ = new uint8_t[size_];
  // initialization, set bits of the first byte to 1
  wheelArray_[0] = 0xff;

  const uint32_t smallPrimes[6] = { 7, 11, 13, 17, 19, 23 };
  // helps to unset 1 bits corresponding to multiples inside the
  // wheelArray_
  const unsigned int unsetBit[37] = { 0xff,
      BIT7, 0xff, 0xff, 0xff, 0xff, 0xff,
      BIT0, 0xff, 0xff, 0xff, BIT1, 0xff,
      BIT2, 0xff, 0xff, 0xff, BIT3, 0xff,
      BIT4, 0xff, 0xff, 0xff, BIT5, 0xff,
      0xff, 0xff, 0xff, 0xff, BIT6, 0xff,
      BIT7, 0xff, 0xff, 0xff, 0xff, 0xff };

  uint32_t primeProduct = 2 * 3 * 5;
  for (uint32_t i = 0; i < 6 && smallPrimes[i] <= limit_; i++) {
    // cross off the multiples of primes < smallPrimes[i] up to the
    // current prime product
    uint32_t pp30 = primeProduct / SieveOfEratosthenes::NUMBERS_PER_BYTE;
    for (uint32_t j = 1; j < smallPrimes[i]; j++) {
      assert((j + 1) * pp30 <= size_);
      std::memcpy(&wheelArray_[j * pp30], wheelArray_, pp30);
    }
    primeProduct *= smallPrimes[i];
    uint32_t multiple = smallPrimes[i];
    // cross off the multiples of smallPrimes[i] up to the current
    // prime product
    // '+ 1' and '- 6' are corrections for primes of type i*30 + 31
    while (multiple <= primeProduct + 1) {
      uint32_t multipleIndex = (multiple - 6) / 30;
      uint32_t bitPosition = multiple - multipleIndex * 30;
      assert(multipleIndex < size_ && 
             bitPosition < 37);
      wheelArray_[multipleIndex] &= unsetBit[bitPosition];
      multiple += smallPrimes[i] * 2;
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
