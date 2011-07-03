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
#include "SieveOfEratosthenes.h"
#include "defs.h"
#include "cpuid.h" 
#include "pmath.h"

#include <algorithm>
#include <cstdlib>
#include <cassert>
#include <iostream>
#include <sstream>

namespace {
  const uint32_t END = 1 << (8 * sizeof(uint8_t));
}

const uint32_t PrimeNumberFinder::nextBitValues_[NUMBERS_PER_BYTE] = { 0,
     0, 0, 0, 0,  0, 0,
    11, 0, 0, 0, 13, 0,
    17, 0, 0, 0, 19, 0,
    23, 0, 0, 0, 29, 0,
     0, 0, 0, 0, 31 };

PrimeNumberFinder::PrimeNumberFinder(PrimeSieve& ps) :
  SieveOfEratosthenes(
      std::max<uint64_t>(ps.getStartNumber(), 7),
      ps.getStopNumber(),
      ps.getSieveSize() * 1024,
      ps.getPreSieveLimit()),
      ps_(ps), kTupletByteCounts_(NULL), kTupletBitValues_(NULL) {
  assert(PrimeSieve::COUNTS_SIZE >= 1 + 6);
#if defined(POPCNT64)
  if ((ps_.flags_ & PrimeSieve::COUNT_PRIMES) != 0 && isPOPCNTSupported())
    ps_.flags_ |= PrimeSieve::SSE4_POPCNT;
#endif
  this->initLookupTables();
}

PrimeNumberFinder::~PrimeNumberFinder() {
  if (kTupletByteCounts_ != NULL) {
    for (uint32_t i = 0; i < 6; i++)
      delete[] kTupletByteCounts_[i];
    delete[] kTupletByteCounts_;
  }
  if (kTupletBitValues_ != NULL) {
    for (uint32_t i = 0; i < 256; i++)
      delete[] kTupletBitValues_[i];
    delete[] kTupletBitValues_;
  }
}

void PrimeNumberFinder::initLookupTables() {
  const uint32_t bitmasks[6][5] = {
      { 0x06, 0x18, 0xc0, END },       // twin prime bitmasks
      { 0x07, 0x0e, 0x1c, 0x38, END }, // prime triplet bitmasks
      { 0x1e, END },                   // prime quadruplet bitmasks
      { 0x1f, 0x3e, END },             // prime quintuplet bitmasks
      { 0x3f, END },                   // prime sextuplet bitmask
      { 0xfe, END } };                 // prime septuplet bitmask

  if (ps_.flags_ & PrimeSieve::COUNT_KTUPLETS) {
    // initialize the lookup tables needed to count prime k-tuplets
    // (twin primes, prime triplets, ...) per byte
    kTupletByteCounts_ = new uint32_t*[6];
    for (uint32_t i = 0; i < 6; i++) {
      kTupletByteCounts_[i] = NULL;
      if (ps_.flags_  & (PrimeSieve::COUNT_TWINS << i)) {
        kTupletByteCounts_[i] = new uint32_t[256];
        for (uint32_t j = 0; j < 256; j++) {
          uint32_t bitmaskCount = 0;
          for (const uint32_t* b = bitmasks[i]; *b <= j; b++) {
            if ((j & *b) == *b)
              bitmaskCount++;
          }
          kTupletByteCounts_[i][j] = bitmaskCount;
        }
      }
    }
  }
  if (ps_.flags_ & PrimeSieve::PRINT_KTUPLETS) {
    // i=0 twins, i=1 triplets, ...
    uint32_t i = 0;
    while ((ps_.flags_ & (PrimeSieve::PRINT_TWINS << i)) == 0)
      i++;
    // initialize the lookup table needed to reconstruct prime
    // k-tuplets from bitmasks of the sieve array
    kTupletBitValues_ = new uint32_t*[256];
    for (uint32_t j = 0; j < 256; j++) {
      kTupletBitValues_[j] = new uint32_t[5];
      uint32_t bitmaskCount = 0;
      for (const uint32_t* b = bitmasks[i]; *b <= j; b++) {
        if ((j & *b) == *b)
          kTupletBitValues_[j][bitmaskCount++] = bitValues_[bitScanForward(*b)];
      }
      kTupletBitValues_[j][bitmaskCount] = END;
    }
  }
}

void PrimeNumberFinder::analyseSieve(const uint8_t* sieve, uint32_t sieveSize) {
  // the C++ Standard guarantees that memory is suitably aligned,
  // see "3.7.3.1 Allocation functions"
  assert(reinterpret_cast<uintptr_t> (sieve) % sizeof(uint32_t) == 0);

  if (ps_.flags_ & PrimeSieve::COUNT_FLAGS)
    this->count(sieve, sieveSize);
  if (ps_.flags_ & PrimeSieve::GENERATE_FLAGS)
    this->generate(sieve, sieveSize);

  uint32_t processed = sieveSize * NUMBERS_PER_BYTE;
  ps_.parent_->doStatus(processed);
}

/**
 * Count the prime numbers and prime k-tuplets of the current
 * segment.
 */
void PrimeNumberFinder::count(const uint8_t* sieve, uint32_t sieveSize) {
  // count prime numbers
  if (ps_.flags_ & PrimeSieve::COUNT_PRIMES) {
    uint32_t primeCount = 0;
#if defined(POPCNT64)
    // count bits using the SSE4.2 POPCNT instruction
    if (ps_.flags_ & PrimeSieve::SSE4_POPCNT) {
      uint32_t i = 0;
      for (; i < sieveSize - sieveSize % 8; i += 8)
        primeCount += POPCNT64(&sieve[i]);
      // count the remaining bytes (MAX 7)
      if (i < sieveSize)
        primeCount += popCount(&sieve[i], sieveSize - i);
    }
    else // no SSE4.2 POPCNT
#endif
    {
      // count bits using BitSlice(24) (see pmath.h)
      primeCount += popCount(reinterpret_cast<const uint32_t*> (sieve), 
          sieveSize / sizeof(uint32_t));
      uint32_t left = sieveSize % sizeof(uint32_t);
      if (left > 0)
        primeCount += popCount(&sieve[sieveSize - left], left);
    }
    // add up to total prime count
    ps_.counts_[0] += primeCount;
  }
  // count prime k-tuplets (i=0 twins, i=1 triplets, ...)
  // using lookup tables
  for (uint32_t i = 0; i < 6; i++) {
    if (ps_.flags_ & (PrimeSieve::COUNT_TWINS << i)) {
      uint32_t kTupletCount = 0;
      for (uint32_t j = 0; j < sieveSize; j++)
        kTupletCount += kTupletByteCounts_[i][sieve[j]];
      ps_.counts_[i+1] += kTupletCount;
    }
  }
}

/**
 * Reconstruct primes numbers from 1 bits of the sieve array and call
 * a callback function for each prime.
 */
#define GENERATE_PRIMES(callback, ...) {                         \
  uint32_t i = 0;                                                \
  for (; i < sieveSize / sizeof(uint32_t); i++) {                \
    uint32_t s32 = reinterpret_cast<const uint32_t*> (sieve)[i]; \
    while (s32 != 0) {                                           \
      uint32_t bitPosition = bitScanForward(s32);                \
      s32 &= s32 - 1;                                            \
      uint64_t prime = lowerBound + bitValues_[bitPosition];     \
      callback (prime, ##__VA_ARGS__);                           \
    }                                                            \
    lowerBound += NUMBERS_PER_BYTE * sizeof(uint32_t);           \
  }                                                              \
  for (i *= sizeof(uint32_t); i < sieveSize; i++) {              \
    uint32_t s = sieve[i];                                       \
    while (s != 0) {                                             \
      uint32_t bitPosition = bitScanForward(s);                  \
      s &= s - 1;                                                \
      uint64_t prime = lowerBound + bitValues_[bitPosition];     \
      callback (prime, ##__VA_ARGS__);                           \
    }                                                            \
    lowerBound += NUMBERS_PER_BYTE;                              \
  }                                                              \
}

/**
 * Generate the prime numbers or prime k-tuplets (twin primes, prime
 * triplets, ...) of the current segment.
 */
void PrimeNumberFinder::generate(const uint8_t* sieve, uint32_t sieveSize) {
  uint64_t lowerBound = this->getSegmentLow();
       if (ps_.flags_ & PrimeSieve::CALLBACK_PRIMES)     GENERATE_PRIMES(ps_.callback_)
  else if (ps_.flags_ & PrimeSieve::CALLBACK_PRIMES_OOP) GENERATE_PRIMES(ps_.callbackOOP_, ps_.cbObj_)
  else if (ps_.flags_ & PrimeSieve::PRINT_PRIMES)        GENERATE_PRIMES(printPrime)
  else {
    // print prime k-tuplets to cout
    for (uint32_t i = 0; i < sieveSize; i++, lowerBound += NUMBERS_PER_BYTE) {
      for (uint32_t* bitValue = kTupletBitValues_[sieve[i]]; *bitValue != END; bitValue++) {
        uint32_t v = *bitValue;
        uint32_t j = 0;
        std::ostringstream kTuplet;
        kTuplet << "(";
        do {
          kTuplet << lowerBound + v << ", ";
          v = nextBitValues_[v];
        } while ((ps_.flags_ & (PrimeSieve::PRINT_TWINS << j++)) == 0);
        kTuplet << lowerBound + v << ")\n";
        std::cout << kTuplet.str();
      }
    }
  }
}

void PrimeNumberFinder::printPrime(uint64_t prime) {
  std::ostringstream out;
  out << prime << '\n';
  std::cout << out.str();
}
