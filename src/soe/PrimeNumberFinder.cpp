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

#include "PrimeNumberFinder.h"
#include "PrimeSieve.h"
#include "SieveOfEratosthenes.h"
#include "defs.h"
#include "bithacks.h"
#include "imath.h"

#include <algorithm>
#include <cstdlib>
#include <cassert>
#include <iostream>
#include <sstream>

const uint32_t PrimeNumberFinder::kTupletBitmasks_[6][5] = {
  { 0x06, 0x18, 0xc0, END },        // Twin primes
  { 0x07, 0x0e, 0x1c, 0x38, END },  // Prime triplets
  { 0x1e, END },                    // Prime quadruplets
  { 0x1f, 0x3e, END },              // Prime quintuplets
  { 0x3f, END },                    // Prime sextuplets
  { 0xfe, END }                     // Prime septuplets
};

PrimeNumberFinder::PrimeNumberFinder(PrimeSieve& ps) :
  SieveOfEratosthenes(
      std::max<uint64_t>(ps.getStartNumber(), 7),
      ps.getStopNumber(),
      ps.getSieveSize() * 1024,
      ps.getPreSieveLimit()),
      ps_(ps), kTupletByteCounts_(NULL) {
  assert(PrimeSieve::COUNTS_SIZE >= 7);
  if (ps_.flags_ & PrimeSieve::COUNT_KTUPLETS)
    this->initLookupTables();
}

PrimeNumberFinder::~PrimeNumberFinder() {
  if (kTupletByteCounts_ != NULL) {
    for (int i = 0; i < 6; i++)
      delete[] kTupletByteCounts_[i];
    delete[] kTupletByteCounts_;
  }
}

/**
 * Initialize the lookup tables needed to count the prime k-tuplets
 * (twin primes, prime triplets, ...) per byte.
 */
void PrimeNumberFinder::initLookupTables() {
  kTupletByteCounts_ = new uint32_t*[6];
  for (uint32_t i = 0; i < 6; i++) {
    kTupletByteCounts_[i] = NULL;
    if (ps_.flags_ & (PrimeSieve::COUNT_TWINS << i)) {
      kTupletByteCounts_[i] = new uint32_t[256];
      for (uint32_t j = 0; j < 256; j++) {
        uint32_t bitmaskCount = 0;
        for (const uint32_t* b = kTupletBitmasks_[i]; *b <= j; b++) {
          if ((j & *b) == *b)
            bitmaskCount++;
        }
        kTupletByteCounts_[i][j] = bitmaskCount;
      }
    }
  }
}

void PrimeNumberFinder::analyseSieve(const uint8_t* sieve, uint32_t sieveSize) {
  if (ps_.flags_ & PrimeSieve::COUNT_FLAGS)
    this->count(sieve, sieveSize);
  if (ps_.flags_ & PrimeSieve::GENERATE_FLAGS)
    this->generate(sieve, sieveSize);
  uint32_t processed = sieveSize * NUMBERS_PER_BYTE;
  ps_.parent_->doStatus(processed);
}

/**
 * Count the prime numbers and prime k-tuplets within the
 * current segment.
 */
void PrimeNumberFinder::count(const uint8_t* sieve, uint32_t sieveSize) {
  // count prime numbers (1 bits within the sieve)
  if (ps_.flags_ & PrimeSieve::COUNT_PRIMES) {
    // see bithacks.h
    uint32_t primeCount = popcount_lauradoux(
        reinterpret_cast<const uint64_t*> (sieve), sieveSize / 8);
    uint32_t bytesLeft = sieveSize % 8;
    if (bytesLeft > 0)
      primeCount += popcount_kernighan(
          &sieve[sieveSize - bytesLeft], bytesLeft);
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
 * Generate the prime numbers or prime k-tuplets (twin primes, prime
 * triplets, ...) of the current segment.
 */
void PrimeNumberFinder::generate(const uint8_t* sieve, uint32_t sieveSize) {
  uint64_t lowerBound = this->getSegmentLow();
  // the GENERATE_PRIMES() macro is defined in defs.h
       if (ps_.flags_ & PrimeSieve::CALLBACK_PRIMES)     GENERATE_PRIMES(ps_.callback_,     uint64_t)
  else if (ps_.flags_ & PrimeSieve::CALLBACK_PRIMES_OOP) GENERATE_PRIMES(this->callbackOOP, uint64_t)
  else if (ps_.flags_ & PrimeSieve::PRINT_PRIMES)        GENERATE_PRIMES(this->printPrime,  uint64_t)
  else if (ps_.flags_ & PrimeSieve::PRINT_KTUPLETS) {
    // i=0 twins, i=1 triplets, ...
    uint32_t i = 0;
    while ((ps_.flags_ & (PrimeSieve::PRINT_TWINS << i)) == 0)
      i++;
    // print prime k-tuplets to cout
    for (uint32_t j = 0; j < sieveSize; j++) {
      for (const uint32_t* bitmasks = kTupletBitmasks_[i]; *bitmasks <= sieve[j]; bitmasks++) {
        if ((sieve[j] & *bitmasks) == *bitmasks) {
          uint32_t leastBit = bitScanForward(*bitmasks);
          std::ostringstream kTuplet;
          kTuplet << "(";
          for (uint32_t l = leastBit; l < leastBit + i + 2; l++) {
            kTuplet << lowerBound + j * NUMBERS_PER_BYTE + bitValues_[l]
                    << (l < leastBit + i + 1 ? ", " : ")\n");
          }
          std::cout << kTuplet.str();
        }
      }
    }
  }
}

void PrimeNumberFinder::callbackOOP(uint64_t prime) {
  ps_.callbackOOP_(prime, ps_.cbObj_);
}

void PrimeNumberFinder::printPrime(uint64_t prime) const {
  std::ostringstream oss;
  oss << prime << '\n';
  std::cout << oss.str();
}
