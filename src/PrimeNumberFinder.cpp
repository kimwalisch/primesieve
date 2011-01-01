/*
 * PrimeNumberFinder.cpp -- This file is part of primesieve
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

#include "PrimeNumberFinder.h"
#include "utils/cpuid.h" 
#include "pmath.h"

#include <stdint.h>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cassert>

namespace {
  const uint32_t END = ~0u;
  const uint32_t BYTE_SIZE = 256;
}

const uint32_t PrimeNumberFinder::nextBitValue_[NUMBERS_PER_BYTE] = { 0,
     0, 0, 0, 0,  0, 0,
    11, 0, 0, 0, 13, 0,
    17, 0, 0, 0, 19, 0,
    23, 0, 0, 0, 29, 0,
     0, 0, 0, 0, 31 };

PrimeNumberFinder::PrimeNumberFinder(uint64_t startNumber, uint64_t stopNumber,
    uint32_t sieveSize, ResetSieve* resetSieve, Results* results,
    uint32_t flags) :
  SieveOfEratosthenes(startNumber, stopNumber, sieveSize, resetSieve), flags_(
      flags), isPOPCNTSupported_(utils::isPOPCNTSupported()), primeByteCounts_(
      NULL), primeBitValues_(NULL), results_(results), status_(0) {
  if (results_ == NULL && (flags_ & RESULTS_FLAGS))
    throw std::logic_error("PrimeNumberFinder: cannot use results_ (is NULL).");
  this->initLookupTables();
}

PrimeNumberFinder::~PrimeNumberFinder() {
  if (primeByteCounts_ != NULL) {
    for (uint32_t i = 0; i < 7; i++) {
      if (primeByteCounts_[i] != NULL)
        delete[] primeByteCounts_[i];
    }
    delete[] primeByteCounts_;
  }
  if (primeBitValues_ != NULL) {
    for (uint32_t i = 0; i < BYTE_SIZE; i++) {
      delete[] primeBitValues_[i];
    }
    delete[] primeBitValues_;
  }
}

/**
 * Initialize lookup tables needed to count and print primes.
 */
void PrimeNumberFinder::initLookupTables() {
  // bits and bitmasks representing the prime numbers and k-tuplets
  // within a byte of the sieve_ array
  const uint32_t bitmasks[7][9] = { { 0x01, 0x02, 0x04, 0x08, 0x10,
        0x20, 0x40, 0x80, END }, // prime numbers
      { 0x06, 0x18, 0xc0, END }, // twin primes
      { 0x07, 0x0e, 0x1c, 0x38, END }, // prime triplets
      { 0x1e, END },            // prime quadruplets
      { 0x1f, 0x3e, END },      // prime quintuplets
      { 0x3f, END },            // prime sextuplets     
      { 0xfe, END } };          // prime septuplets

  if (flags_ & COUNT_FLAGS) {
    primeByteCounts_ = new uint32_t*[7];
    for (uint32_t i = 0; i < 7; i++) {
      if ((flags_  & (COUNT_PRIMES << i)) == 0) 
        primeByteCounts_[i] = NULL;
      else {
        primeByteCounts_[i] = new uint32_t[BYTE_SIZE];
        // save the count of bitmasks within each byte value
        for (uint32_t j = 0; j < BYTE_SIZE; j++) {
          uint32_t bitmaskCount = 0;
          for (const uint32_t* b = bitmasks[i]; *b != END; b++) {
            if ((j & *b) == *b)
              bitmaskCount++;
          }
          primeByteCounts_[i][j] = bitmaskCount;
        }
      }
    }
  }
  if (flags_ & PRINT_FLAGS) {
    uint32_t index = 0;
    for (uint32_t i = PRINT_PRIMES; (i & flags_) == 0; i <<= 1)
      index++;
    primeBitValues_ = new uint32_t*[BYTE_SIZE];
    // generate the bitValues for each byte value
    for (uint32_t i = 0; i < BYTE_SIZE; i++) {
      primeBitValues_[i] = new uint32_t[9];
      uint32_t bitmaskCount = 0;
      // save the bitValues of the current byte value
      for (const uint32_t* b = bitmasks[index]; *b != END; b++) {
        if ((i & *b) == *b) {
          primeBitValues_[i][bitmaskCount] = bitValues_[ntz(*b)];
          bitmaskCount++;
        }
      }
      primeBitValues_[i][bitmaskCount] = END;
    }
  }
}

/**
 * Count the prime numbers and prime k-tuplets of the current
 * sieve round.
 */
void PrimeNumberFinder::count(const uint8_t* sieve, uint32_t sieveSize) {
  // count prime numbers
  if (flags_ & COUNT_PRIMES) {
    uint32_t primeCount = 0;
    uint32_t i = 0;
#if defined(POPCNT64)
    // count bits using the SSE 4.2 POPCNT instruction
    if (isPOPCNTSupported_) {
      for (; i + 8 < sieveSize; i += 8)
        primeCount += POPCNT64(sieve, i);
    }
#endif
    // count bits using a lookup table
    for (; i < sieveSize; i++)
      primeCount += primeByteCounts_[0][sieve[i]];
    results_->counts[0] += primeCount;
  }
  // count prime k-tuplets
  for (uint32_t i = 1; i < results_->COUNTS_SIZE; i++) {
    if (flags_  & (COUNT_PRIMES << i)) {
      uint32_t kTupletCount = 0;
      for (uint32_t j = 0; j < sieveSize; j++) {
        kTupletCount += primeByteCounts_[i][sieve[j]];
      }
      results_->counts[i] += kTupletCount;
    }
  }
}

/**
 * Print the status (in percents) of the sieving process
 * to the standard output.
 */
void PrimeNumberFinder::status(uint32_t sieveSize) {
  uint64_t upperBound = this->getLowerBound() + sieveSize
      * NUMBERS_PER_BYTE + 1;
  float status = 100.0f;
  if (upperBound < this->getStopNumber()) {
    status *= 1.0f - 
        static_cast<float> (this->getStopNumber() - upperBound) /
        static_cast<float> (this->getStopNumber() - this->getStartNumber());
  }
  if (flags_ & STORE_STATUS)
    results_->status = status;
  if (flags_ & PRINT_STATUS) {
    uint32_t floorStatus = static_cast<uint32_t> (status);
    if (status_ < floorStatus) {
      status_ = floorStatus;
      std::cout << '\r' << status_ << '%' << std::flush;
    }
  }
}

/**
 * Print the prime numbers or prime k-tuplets of the current
 * sieve round to the standard output.
 */
void PrimeNumberFinder::print(const uint8_t* sieve, uint32_t sieveSize) {
  uint64_t byteValue = this->getLowerBound();
  for (uint32_t i = 0; i < sieveSize; i++) {
    for (uint32_t* bitValue = primeBitValues_[sieve[i]]; *bitValue != END; bitValue++) {
      if (flags_ & PRINT_PRIMES)
        // print the current prime number
        std::cout << byteValue + *bitValue << std::endl;
      else {
        // print the current prime k-tuplet
        std::ostringstream kTuplet;
        kTuplet << "(";
        uint32_t v = *bitValue;
        for (uint32_t j = PRINT_PRIMES; (j & flags_) == 0; j <<= 1) {
          kTuplet << byteValue + v << ", ";
          v = nextBitValue_[v];
        }
        kTuplet << byteValue + v << ")";
        std::cout << kTuplet.str() << std::endl;
      }
    }
    byteValue += NUMBERS_PER_BYTE;
  }
}

void PrimeNumberFinder::analyseSieve(const uint8_t* sieve,
    uint32_t sieveSize) {
  if (flags_ & COUNT_FLAGS)
    this->count(sieve, sieveSize);
  if (flags_ & STATUS_FLAGS)
    this->status(sieveSize);
  if (flags_ & PRINT_FLAGS)
    this->print(sieve, sieveSize);
}
