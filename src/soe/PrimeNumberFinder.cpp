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

#include "PrimeNumberFinder.h"
#include "PrimeSieve.h"
#include "SieveOfEratosthenes.h"
#include "config.h"
#include "bithacks.h"

#include <stdint.h>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>

namespace soe {

/** Bit patterns corresponding to prime k-tuplets */
const uint32_t PrimeNumberFinder::kTupletBitmasks_[6][5] =
{
  { 0x06, 0x18, 0xc0, END },       // Twin primes
  { 0x07, 0x0e, 0x1c, 0x38, END }, // Prime triplets
  { 0x1e, END },                   // Prime quadruplets
  { 0x1f, 0x3e, END },             // Prime quintuplets
  { 0x3f, END },                   // Prime sextuplets
  { 0xfe, END }                    // Prime septuplets
};

PrimeNumberFinder::PrimeNumberFinder(PrimeSieve& ps) :
  SieveOfEratosthenes(
      std::max<uint64_t>(7, ps.getStart()),
      ps.getStop(),
      ps.getPreSieveLimit(),
      ps.getSieveSize()),
  ps_(ps),
  kTupletByteCounts_(NULL)
{
  if (ps_.testFlags(ps_.COUNT_KTUPLETS))
    initLookupTables();
}

PrimeNumberFinder::~PrimeNumberFinder() {
  if (kTupletByteCounts_ != NULL) {
    for (int i = 0; i < 6; i++)
      delete[] kTupletByteCounts_[i];
    delete[] kTupletByteCounts_;
  }
}

/**
 * Check if PrimeNumberFinder requires a PrimeNumberGenerator
 * object to generate its sieving primes.
 */
bool PrimeNumberFinder::needGenerator() const {
  return getPreSieveLimit() < getSquareRoot();
}

/**
 * Initialize the lookup tables needed to count prime k-tuplets
 * (twin primes, prime triplets, ...) per byte.
 */
void PrimeNumberFinder::initLookupTables() {
  kTupletByteCounts_ = new uint32_t*[6];
  for (uint32_t i = 0; i < 6; i++) {
    kTupletByteCounts_[i] = NULL;
    if (ps_.isFlag(ps_.COUNT_TWINS << i)) {
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

/**
 * Executed after each sieved segment, generates and counts the primes
 * (1 bits within sieve array) within the interval
 * [segmentLow_+7, segmentHigh_].
 * @see SieveOfEratosthenes::sieve(uint32_t)
 */
void PrimeNumberFinder::segmentProcessed(const uint8_t* sieve, uint32_t sieveSize) {
  if (ps_.testFlags(ps_.COUNT_FLAGS))
    count(sieve, sieveSize);
  if (ps_.testFlags(ps_.GENERATE_FLAGS))
    generate(sieve, sieveSize);
  if (ps_.isFlag(ps_.CALCULATE_STATUS))
    ps_.calcStatus(sieveSize * NUMBERS_PER_BYTE);
}

/**
 * Count the primes and prime k-tuplets within
 * the current segment.
 */
void PrimeNumberFinder::count(const uint8_t* sieve, uint32_t sieveSize) {
  // count prime numbers (1 bits within sieve array)
  if (ps_.isFlag(ps_.COUNT_PRIMES)) {
    const uint64_t* sieve64 = reinterpret_cast<const uint64_t*>(sieve);
    uint32_t sieveSize64 = sieveSize / 8;
    uint32_t bytesLeft = sieveSize % 8;
    // see bithacks.h
    uint32_t primeCount = popcount_lauradoux(sieve64, sieveSize64);
    if (bytesLeft > 0)
      primeCount += popcount_kernighan(&sieve[sieveSize - bytesLeft], bytesLeft);
    // add up to total prime count
    ps_.counts_[0] += primeCount;
  }
  // count prime k-tuplets (i=0 twins, i=1 triplets, ...)
  // using lookup tables
  for (uint32_t i = 0; i < 6; i++) {
    if (ps_.isFlag(ps_.COUNT_TWINS << i)) {
      uint32_t kCount = 0;
      for (uint32_t j = 0; j < sieveSize; j++)
        kCount += kTupletByteCounts_[i][sieve[j]];
      ps_.counts_[i+1] += kCount;
    }
  }
}

/**
 * Generate the primes or prime k-tuplets (twin primes, prime
 * triplets, ...) within the current segment.
 */
void PrimeNumberFinder::generate(const uint8_t* sieve, uint32_t sieveSize) {
  if (ps_.testFlags(ps_.PRINT_KTUPLETS)) {
    const uint64_t segmentLow = getSegmentLow();
    // i = 0 twins, i = 1 triplets, ...
    uint32_t i = 0;
    for (; !ps_.isFlag(ps_.PRINT_TWINS << i); i++)
      ;
    // print prime k-tuplets to std::cout
    for (uint32_t j = 0; j < sieveSize; j++) {
      for (const uint32_t* bitmask = kTupletBitmasks_[i]; *bitmask <= sieve[j]; bitmask++) {
        if ((sieve[j] & *bitmask) == *bitmask) {
          std::ostringstream kTuplet;
          kTuplet << "(";
          uint32_t bits = *bitmask;
          uint64_t offset = segmentLow + j * NUMBERS_PER_BYTE;
          for (; bits & (bits - 1); bits &= bits - 1)
            kTuplet << offset + getFirstSetBitValue(bits) << ", ";
          kTuplet << offset + getFirstSetBitValue(bits) << ")\n";
          std::cout << kTuplet.str();
        }
      }
    }
  }
  else {
    // to keep things simple only one thread at a time calls back primes
    PrimeSieve::LockGuard lock(ps_);
    // GENERATE_PRIMES() is defined in SieveOfEratosthenes.h
    if (ps_.isFlag(ps_.CALLBACK32_PRIMES))     GENERATE_PRIMES(ps_.callback32_, uint32_t)
    if (ps_.isFlag(ps_.CALLBACK64_PRIMES))     GENERATE_PRIMES(ps_.callback64_, uint64_t)
    if (ps_.isFlag(ps_.CALLBACK32_OOP_PRIMES)) GENERATE_PRIMES(callback32_OOP,  uint32_t)
    if (ps_.isFlag(ps_.CALLBACK64_OOP_PRIMES)) GENERATE_PRIMES(callback64_OOP,  uint64_t)
    if (ps_.isFlag(ps_.PRINT_PRIMES))          GENERATE_PRIMES(print,           uint64_t)
  }
}

void PrimeNumberFinder::callback32_OOP(uint32_t prime) const {
  ps_.callback32_OOP_(prime, ps_.cbObj_);
}

void PrimeNumberFinder::callback64_OOP(uint64_t prime) const {
  ps_.callback64_OOP_(prime, ps_.cbObj_);
}

void PrimeNumberFinder::print(uint64_t prime) {
  std::cout << prime << '\n';
}

} // namespace soe
