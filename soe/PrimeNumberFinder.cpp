/*
 * PrimeNumberFinder.cpp -- This file is part of primesieve
 *
 * Copyright (C) 2011 Kim Walisch, <kim.walisch@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include "PrimeNumberFinder.h"
#include "PrimeSieve.h"
#include "defs.h"
#include "cpuid.h" 
#include "pmath.h"

#include <cstdlib>
#include <iostream>

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

PrimeNumberFinder::PrimeNumberFinder(PrimeSieve* primeSieve, 
    ResetSieve& resetSieve) :
  SieveOfEratosthenes(
     (primeSieve->startNumber_ < 7) ?7 :primeSieve->startNumber_,
      primeSieve->stopNumber_,
      primeSieve->sieveSize_,
      &resetSieve),
      primeSieve_(primeSieve), primeByteCounts_(NULL), primeBitValues_(NULL),
      isPOPCNTSupported_(isPOPCNTSupported()) {
  this->initLookupTables();
}

PrimeNumberFinder::~PrimeNumberFinder() {
  if (primeByteCounts_ != NULL) {
    for (uint32_t i = 0; i < COUNTS_SIZE; i++) {
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
  const uint32_t bitmasks[COUNTS_SIZE][9] = {
      { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, END }, // prime numbers
      { 0x06, 0x18, 0xc0, END }, // twin primes
      { 0x07, 0x0e, 0x1c, 0x38, END }, // prime triplets
      { 0x1e, END },            // prime quadruplets
      { 0x1f, 0x3e, END },      // prime quintuplets
      { 0x3f, END },            // prime sextuplets     
      { 0xfe, END } };          // prime septuplets

  if (primeSieve_->flags_ & COUNT_FLAGS) {
    primeByteCounts_ = new uint32_t*[COUNTS_SIZE];
    for (uint32_t i = 0; i < COUNTS_SIZE; i++) {
      if ((primeSieve_->flags_  & (COUNT_PRIMES << i)) == 0) 
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
  if (primeSieve_->flags_ & GENERATE_FLAGS) {
    uint32_t generateType = 0;
    if (primeSieve_->flags_ & PRINT_FLAGS)
      for (uint32_t i = PRINT_PRIMES; (i & primeSieve_->flags_) == 0; i <<= 1)
        generateType++;
    primeBitValues_ = new uint32_t*[BYTE_SIZE];
    // generate the bitValues for each byte value
    for (uint32_t i = 0; i < BYTE_SIZE; i++) {
      primeBitValues_[i] = new uint32_t[9];
      uint32_t bitmaskCount = 0;
      // save the bitValues of the current byte value
      for (const uint32_t* b = bitmasks[generateType]; *b != END; b++) {
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
 * Count the prime numbers and prime k-tuplets of the current segment.
 */
void PrimeNumberFinder::count(const uint8_t* sieve, uint32_t sieveSize) {
  // count prime numbers
  if (primeSieve_->flags_ & COUNT_PRIMES) {
    uint32_t primeCount = 0;
    uint32_t i = 0;
#if defined(POPCNT64)
    // count bits using the SSE 4.2 POPCNT instruction
    if (isPOPCNTSupported_)
      for (; i + 8 < sieveSize; i += 8)
        primeCount += POPCNT64(sieve, i);
#endif
    // count bits using a lookup table
    for (; i < sieveSize; i++)
      primeCount += primeByteCounts_[0][sieve[i]];
    primeSieve_->counts_[0] += primeCount;
  }
  // count prime k-tuplets
  for (uint32_t i = 1; i < COUNTS_SIZE; i++) {
    if (primeSieve_->flags_  & (COUNT_PRIMES << i)) {
      uint32_t kTupletCount = 0;
      for (uint32_t j = 0; j < sieveSize; j++) {
        kTupletCount += primeByteCounts_[i][sieve[j]];
      }
      primeSieve_->counts_[i] += kTupletCount;
    }
  }
}

/**
 * Reconstructs prime numbers or prime k-tuplets (twin primes, prime
 * triplets, ...) from 1 bits after that all multiples have been
 * crossed off in the sieve array.
 */
void PrimeNumberFinder::generate(const uint8_t* sieve, uint32_t sieveSize) {
  uint64_t byteValue = this->getLowerBound();
  // call a callback function for each prime number
  if (primeSieve_->flags_ & CALLBACK_PRIMES_IMP)
    for (uint32_t i = 0; i < sieveSize; i++, byteValue += NUMBERS_PER_BYTE)
      for (uint32_t* bitValue = primeBitValues_[sieve[i]]; *bitValue != END; bitValue++)
        primeSieve_->callback_imp(byteValue + *bitValue);
  else if (primeSieve_->flags_ & CALLBACK_PRIMES_OOP)
    for (uint32_t i = 0; i < sieveSize; i++, byteValue += NUMBERS_PER_BYTE)
      for (uint32_t* bitValue = primeBitValues_[sieve[i]]; *bitValue != END; bitValue++)
        primeSieve_->callback_oop(byteValue + *bitValue, primeSieve_->cbObj_);
  // print the prime numbers to stdout
  else if (primeSieve_->flags_ & PRINT_PRIMES)
    for (uint32_t i = 0; i < sieveSize; i++, byteValue += NUMBERS_PER_BYTE)
      for (uint32_t* bitValue = primeBitValues_[sieve[i]]; *bitValue != END; bitValue++)
        std::cout << byteValue + *bitValue << '\n';
  // print prime k-tuplets (twin primes, prime triplets, ...) to stdout
  else 
    for (uint32_t i = 0; i < sieveSize; i++, byteValue += NUMBERS_PER_BYTE)
      for (uint32_t* bitValue = primeBitValues_[sieve[i]]; *bitValue != END; bitValue++) {
        std::cout << '(';
        uint32_t v = *bitValue;
        for (uint32_t j = PRINT_PRIMES; (j & primeSieve_->flags_) == 0; j <<= 1) {
          std::cout << byteValue + v << ", ";
          v = nextBitValue_[v];
        }
        std::cout << byteValue + v << ")\n";
      }
}

void PrimeNumberFinder::analyseSieve(const uint8_t* sieve,
    uint32_t sieveSize) {
  if (primeSieve_->flags_ & COUNT_FLAGS)
    this->count(sieve, sieveSize);
  if (primeSieve_->flags_ & GENERATE_FLAGS)
    this->generate(sieve, sieveSize);
  primeSieve_->parent_->doStatus(sieveSize * NUMBERS_PER_BYTE);
}
