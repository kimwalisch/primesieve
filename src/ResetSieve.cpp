/*
 * ResetSieve.cpp -- This file is part of primesieve
 *
 * Copyright (C) 2010 Kim Walisch, <kim.walisch@gmail.com>
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
#include "SieveOfEratosthenes.h"
#include "pmath.h"
#include "bits.h"

#include <stdint.h>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <cassert>

ResetSieve::ResetSieve(uint32_t eliminateUpTo) :
  eliminateUpTo_(eliminateUpTo), resetBuffer_(NULL), size_(0) {
  if (eliminateUpTo_ < 7 || eliminateUpTo_ > 23)
    throw std::invalid_argument(
        "ResetSieve: eliminateUpTo must be >= 7 && <= 23.");
  this->setSize(eliminateUpTo_);
  this->initResetBuffer();
}

ResetSieve::~ResetSieve() {
  if (resetBuffer_ != NULL)
    delete[] resetBuffer_;
}

/**
 * The reset index is needed to map the sieve_ array to the
 * resetBuffer_ array in @code reset(uint8_t*, uint32_t, uint32_t*)
 * @endcode
 */
uint32_t ResetSieve::getResetIndex(uint64_t lowerBound) const {
  return static_cast<uint32_t> (lowerBound % primeProduct(eliminateUpTo_))
      / SieveOfEratosthenes::NUMBERS_PER_BYTE;
}

void ResetSieve::setSize(uint32_t eliminateUpTo) {
  assert(eliminateUpTo > 0);
  size_ = primeProduct(eliminateUpTo) / SieveOfEratosthenes::NUMBERS_PER_BYTE;
}

/**
 * Allocates the resetBuffer_ array and eliminates the multiples
 * of prime numbers <= eliminateUpTo_ in it.
 */
void ResetSieve::initResetBuffer() {
  assert(size_ > 0);
  resetBuffer_ = new uint8_t[size_];
  // initialization, set bits of the first byte to 1
  resetBuffer_[0] = 0xff;
  
  const uint32_t smallPrimes[6] = { 7, 11, 13, 17, 19, 23 };
  // helps to unset bits (corresponding to multiples)
  const uint32_t eliminateMultiple[37] = { ~0u,
      BIT7, ~0u, ~0u, ~0u,  ~0u, ~0u,
      BIT0, ~0u, ~0u, ~0u, BIT1, ~0u,
      BIT2, ~0u, ~0u, ~0u, BIT3, ~0u,
      BIT4, ~0u, ~0u, ~0u, BIT5, ~0u,
       ~0u, ~0u, ~0u, ~0u, BIT6, ~0u,
      BIT7, ~0u, ~0u, ~0u,  ~0u, ~0u };

  uint32_t primeProduct = 2 * 3 * 5;
  for (uint32_t i = 0; i < 6 && smallPrimes[i] <= eliminateUpTo_; i++) {
    // eliminate the multiples of the prime numbers <
    // smallPrimes[i] up to its prime product
    uint32_t pp30 = primeProduct / SieveOfEratosthenes::NUMBERS_PER_BYTE;
    for (uint32_t j = 1; j < smallPrimes[i]; j++) {
      assert((j + 1) * pp30 <= size_);
      std::memcpy(&resetBuffer_[j * pp30], resetBuffer_, pp30);
    }
    primeProduct *= smallPrimes[i];
    uint32_t multiple = smallPrimes[i];
    /// cross-off the multiples of smallPrimes[i] up to its 
    /// prime product
    /// @remark '+ 1' is a correction for primes of type n * 30 + 31
    while (multiple <= primeProduct + 1) {
      uint32_t index = (multiple - 6) / SieveOfEratosthenes::NUMBERS_PER_BYTE;
      uint32_t modulo = multiple - index
          * SieveOfEratosthenes::NUMBERS_PER_BYTE;
      assert(index < size_);
      resetBuffer_[index] &= eliminateMultiple[modulo];
      multiple += smallPrimes[i] * 2;
    }
  }
}

/**
 * Used to reset (set bits to 1) the SieveOfEratosthenes::sieve_
 * array after each sieve round, eliminates the multiples of prime
 * numbers <= eliminateUpTo_ without sieving.
 */
void ResetSieve::reset(uint8_t* sieve, uint32_t sieveSize, uint32_t* resetIndex) {
  uint32_t sizeLeft = size_ - *resetIndex;
  if (sizeLeft > sieveSize) {
    // reset the sieve array at once
    std::memcpy(sieve, &resetBuffer_[*resetIndex], sieveSize);
    *resetIndex += sieveSize;
  } else {
    // copy the last remaing bytes at the end of
    // resetBuffer_ to the sieve array
    std::memcpy(sieve, &resetBuffer_[*resetIndex], sizeLeft);
    // restart copying at &resetBuffer_[0].
    uint32_t sieveIndex = sizeLeft;
    while (sieveIndex + size_ < sieveSize) {
      std::memcpy(&sieve[sieveIndex], resetBuffer_, size_);
      sieveIndex += size_;
    }
    // set *resetIndex for the next sieve round
    *resetIndex = sieveSize - sieveIndex;
    std::memcpy(&sieve[sieveIndex], resetBuffer_, *resetIndex);
  }
}
