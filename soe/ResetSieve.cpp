/*
 * ResetSieve.cpp -- This file is part of primesieve
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

#include "ResetSieve.h"
#include "PrimeSieve.h"
#include "defs.h"
#include "SieveOfEratosthenes.h"
#include "pmath.h"

#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <cassert>

ResetSieve::ResetSieve(PrimeSieve* ps) :
    limit_(defs::LIMIT_RESETSIEVE), wheelArray_(NULL), size_(0) {
  if (defs::LIMIT_RESETSIEVE < 13 || defs::LIMIT_RESETSIEVE > 23)
    throw std::logic_error(
        "defs::LIMIT_RESETSIEVE must be >= 13 && <= 23.");
  uint64_t interval = ps->getStopNumber() - ps->getStartNumber();
  uint32_t sqrtStop = isqrt(ps->getStopNumber());
  // if the sieve interval is small use limit_ 13 instead of 19
  if (interval < static_cast<uint64_t> (1E8) && 
      sqrtStop < static_cast<uint64_t> (1E8))
    limit_ = 13;
  this->setSize(limit_);
  this->initWheelArray();
}

ResetSieve::~ResetSieve() {
  delete[] wheelArray_;
}

/**
 * The reset index indicates the wheel position of the sieve array
 * within the wheelArray_.
 */
uint32_t ResetSieve::getResetIndex(uint64_t segmentLow) const {
  return static_cast<uint32_t> (segmentLow % primeProduct(limit_))
      / SieveOfEratosthenes::NUMBERS_PER_BYTE;
}

void ResetSieve::setSize(uint32_t limit) {
  size_ = primeProduct(limit) / SieveOfEratosthenes::NUMBERS_PER_BYTE;
}

/**
 * Allocate the wheelArray_ and remove the multiples of small
 * primes <= limit_ from it.
 */
void ResetSieve::initWheelArray() {
  assert(size_ > 0);
  wheelArray_ = new uint8_t[size_];
  // initialization, set bits of the first byte to 1
  wheelArray_[0] = 0xff;
  
  const uint32_t smallPrimes[6] = { 7, 11, 13, 17, 19, 23 };
  // helps to unset the bits corresponding to multiples inside the
  // wheelArray_
  const unsigned int crossOff[37] = { ~0u,
      BIT7, ~0u, ~0u, ~0u,  ~0u, ~0u,
      BIT0, ~0u, ~0u, ~0u, BIT1, ~0u,
      BIT2, ~0u, ~0u, ~0u, BIT3, ~0u,
      BIT4, ~0u, ~0u, ~0u, BIT5, ~0u,
       ~0u, ~0u, ~0u, ~0u, BIT6, ~0u,
      BIT7, ~0u, ~0u, ~0u,  ~0u, ~0u };

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
      uint32_t index = (multiple - 6) / SieveOfEratosthenes::NUMBERS_PER_BYTE;
      uint32_t bit = multiple - index * SieveOfEratosthenes::NUMBERS_PER_BYTE;
      assert(index < size_);
      wheelArray_[index] &= crossOff[bit];
      multiple += smallPrimes[i] * 2;
    }
  }
}

/**
 * Resets the sieve array (resets bits to 1) of SieveOfEratosthenes
 * objects after each sieved segment and removes the multiples of
 * small primes <= limit_ without sieving.
 * @see SieveOfEratosthenes::sieve(uint32_t)
 */
void ResetSieve::reset(uint8_t* sieve, uint32_t sieveSize, 
    uint32_t* resetIndex) {
  uint32_t sizeLeft = size_ - *resetIndex;
  if (sizeLeft > sieveSize) {
    // reset the sieve array at once
    std::memcpy(sieve, &wheelArray_[*resetIndex], sieveSize);
    *resetIndex += sieveSize;
  } else {
    // copy the last remaining bytes at the end of wheelArray_ to the
    // beginning of the sieve array
    std::memcpy(sieve, &wheelArray_[*resetIndex], sizeLeft);
    // restart copying at the beginning of wheelArray_
    uint32_t sieveIndex = sizeLeft;
    while (sieveIndex + size_ < sieveSize) {
      std::memcpy(&sieve[sieveIndex], wheelArray_, size_);
      sieveIndex += size_;
    }
    // set the resetIndex for the next segment
    *resetIndex = sieveSize - sieveIndex;
    std::memcpy(&sieve[sieveIndex], wheelArray_, *resetIndex);
  }
}
