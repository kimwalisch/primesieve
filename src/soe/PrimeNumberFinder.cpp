///
/// @file   PrimeNumberFinder.cpp
/// @brief  Generates, counts and prints primes and prime k-tuplets
///         (twin primes, prime triplets, ...).
///
/// Copyright (C) 2012 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the New BSD License. See the
/// LICENSE file in the top level directory.
///

#include "config.h"
#include "PrimeNumberFinder.h"
#include "SieveOfEratosthenes.h"
#include "SieveOfEratosthenes-inline.h"
#include "PrimeSieve.h"
#include "SynchronizeThreads.h"
#include "popcount.h"
#include "GENERATE.h"

#include <stdint.h>
#include <algorithm>
#include <vector>
#include <iostream>
#include <sstream>

namespace soe {

const uint_t PrimeNumberFinder::kBitmasks_[7][5] =
{
  { END },
  { 0x06, 0x18, 0xc0, END },       // Twin prime       bitmasks, i.e. b00000110, b00011000, b11000000
  { 0x07, 0x0e, 0x1c, 0x38, END }, // Prime triplet    bitmasks, i.e. b00000111, b00001110, ...
  { 0x1e, END },                   // Prime quadruplet bitmasks
  { 0x1f, 0x3e, END },             // Prime quintuplet bitmasks
  { 0x3f, END },                   // Prime sextuplet  bitmasks
  { 0xfe, END }                    // Prime septuplet  bitmasks
};

PrimeNumberFinder::PrimeNumberFinder(PrimeSieve& ps) :
  SieveOfEratosthenes(
    std::max<uint64_t>(7, ps.getStart()),
    ps.getStop(),
    ps.getSieveSize(),
    ps.getPreSieve()),
  ps_(ps)
{
  if (ps_.isFlag(ps_.COUNT_TWINS, ps_.COUNT_SEPTUPLETS))
    init_kCounts();
}

/// Calculate the number of twins, triplets, ... (bitmask matches)
/// for each possible byte value 0 - 255.
///
void PrimeNumberFinder::init_kCounts()
{
  for (uint_t i = 1; i < ps_.counts_.size(); i++) {
    if (ps_.isCount(i)) {
      kCounts_[i].resize(256);
      for (uint_t j = 0; j < kCounts_[i].size(); j++) {
        uint_t bitmaskCount = 0;
        for (const uint_t* b = kBitmasks_[i]; *b <= j; b++) {
          if ((j & *b) == *b)
            bitmaskCount++;
        }
        kCounts_[i][j] = bitmaskCount;
      }
    }
  }
}

/// Executed after each sieved segment.
/// @see sieveSegment() in SieveOfEratosthenes.cpp
///
void PrimeNumberFinder::segmentProcessed(const uint8_t* sieve, uint_t sieveSize)
{
  if (ps_.isCount())
    count(sieve, sieveSize);
  if (ps_.isGenerate())
    generate(sieve, sieveSize);
  if (ps_.isStatus())
    ps_.updateStatus(sieveSize * NUMBERS_PER_BYTE, /* waitForLock = */ false);
}

/// Count the primes and prime k-tuplets within
/// the current segment.
///
void PrimeNumberFinder::count(const uint8_t* sieve, uint_t sieveSize)
{
  std::vector<uint64_t>& counts = ps_.counts_;
  // count prime numbers (1 bits), see popcount.cpp
  if (ps_.isFlag(ps_.COUNT_PRIMES))
    counts[0] += popcount(reinterpret_cast<const uint64_t*>(sieve), (sieveSize + 7) / 8);
  // count prime k-tuplets (i = 1 twins, i = 2 triplets, ...)
  for (uint_t i = 1; i < counts.size(); i++) {
    if (ps_.isCount(i)) {
      const std::vector<uint_t>& kCounts = kCounts_[i];
      uint_t sum0 = 0;
      uint_t sum1 = 0;
      uint_t sum2 = 0;
      uint_t sum3 = 0;
      for (uint_t j = 0; j < sieveSize; j += 4) {
        sum0 += kCounts[sieve[j+0]];
        sum1 += kCounts[sieve[j+1]];
        sum2 += kCounts[sieve[j+2]];
        sum3 += kCounts[sieve[j+3]];
      }
      counts[i] += (sum0 + sum1) + (sum2 + sum3);
    }
  }
}

/// Generate (print and callback) the primes and prime
/// k-tuplets within the current segment.
/// @warning primes < 7 are handled in PrimeSieve::doSmallPrime()
///
void PrimeNumberFinder::generate(const uint8_t* sieve, uint_t sieveSize) const
{
  // print prime k-tuplets to cout
  if (ps_.isFlag(ps_.PRINT_TWINS, ps_.PRINT_SEPTUPLETS)) {
    uint_t i = 1; // i = 1 twins, i = 2 triplets, ...
    for (; !ps_.isPrint(i); i++)
      ;
    // for more speed see GENERATE.h
    for (uint_t j = 0; j < sieveSize; j++) {
      for (const uint_t* bitmask = kBitmasks_[i]; *bitmask <= sieve[j]; bitmask++) {
        if ((sieve[j] & *bitmask) == *bitmask) {
          std::ostringstream kTuplet;
          kTuplet << "(";
          uint64_t bits = *bitmask;
          while (bits != 0) {
            kTuplet << getNextPrime(&bits, j);
            kTuplet << ((bits != 0) ? ", " : ")\n");
          }
          std::cout << kTuplet.str();
        }
      }
    }
  }
  // callback prime numbers
  if (ps_.isFlag(ps_.PRINT_PRIMES))   { SynchronizeThreads lock(ps_); GENERATE_PRIMES(print,           uint64_t) }
  if (ps_.isFlag(ps_.CALLBACK32))     { SynchronizeThreads lock(ps_); GENERATE_PRIMES(ps_.callback32_, uint32_t) }
  if (ps_.isFlag(ps_.CALLBACK64))     { SynchronizeThreads lock(ps_); GENERATE_PRIMES(ps_.callback64_, uint64_t) }
  if (ps_.isFlag(ps_.CALLBACK32_OBJ)) { SynchronizeThreads lock(ps_); GENERATE_PRIMES(callback32_obj,  uint32_t) }
  if (ps_.isFlag(ps_.CALLBACK64_OBJ)) { SynchronizeThreads lock(ps_); GENERATE_PRIMES(callback64_obj,  uint64_t) }
  if (ps_.isFlag(ps_.CALLBACK64_INT)) {                               GENERATE_PRIMES(callback64_int,  uint64_t) }
}

void PrimeNumberFinder::print(uint64_t prime)
{
  std::cout << prime << '\n';
}

void PrimeNumberFinder::callback32_obj(uint32_t prime) const
{
  ps_.callback32_obj_(prime, ps_.obj_);
}

void PrimeNumberFinder::callback64_obj(uint64_t prime) const
{
  ps_.callback64_obj_(prime, ps_.obj_);
}

void PrimeNumberFinder::callback64_int(uint64_t prime) const
{
  ps_.callback64_int_(prime, ps_.threadNumber_);
}

} // namespace soe
