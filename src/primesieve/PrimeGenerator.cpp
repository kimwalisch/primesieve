///
/// @file   PrimeGenerator.cpp
/// @brief  After a segment has been sieved PrimeGenerator is
///         used to reconstruct primes and prime k-tuplets from
///         1 bits of the sieve array.
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/config.hpp>
#include <primesieve/littleendian_cast.hpp>
#include <primesieve/pmath.hpp>
#include <primesieve/PrimeGenerator.hpp>
#include <primesieve/PrimeSieve.hpp>
#include <primesieve/SieveOfEratosthenes.hpp>
#include <primesieve/StorePrimes.hpp>

#include <stdint.h>
#include <algorithm>
#include <iostream>
#include <sstream>

using namespace std;

namespace primesieve {

/// popcount.cpp
uint64_t popcount(const uint64_t* array, uint64_t size);

const uint64_t PrimeGenerator::bitmasks_[6][5] =
{
  { END },
  { 0x06, 0x18, 0xc0, END },       // Twin primes:       b00000110, b00011000, b11000000
  { 0x07, 0x0e, 0x1c, 0x38, END }, // Prime triplets:    b00000111, b00001110, ...
  { 0x1e, END },                   // Prime quadruplets: b00011110
  { 0x1f, 0x3e, END },             // Prime quintuplets
  { 0x3f, END }                    // Prime sextuplets
};

PrimeGenerator::PrimeGenerator(PrimeSieve& ps, const PreSieve& preSieve) :
  SieveOfEratosthenes(max<uint64_t>(7, ps.getStart()),
                      ps.getStop(),
                      ps.getSieveSize(),
                      preSieve),
  ps_(ps),
  counts_(ps_.getCounts())
{
  if (ps_.isFlag(ps_.COUNT_TWINS, ps_.COUNT_SEXTUPLETS))
    init_kCounts();
}

/// Calculate the number of twins, triplets, ...
/// for each possible byte value
///
void PrimeGenerator::init_kCounts()
{
  for (uint_t i = 1; i < counts_.size(); i++)
  {
    if (ps_.isCount(i))
    {
      kCounts_[i].resize(256);

      for (uint64_t j = 0; j < 256; j++)
      {
        byte_t count = 0;
        for (const uint64_t* b = bitmasks_[i]; *b <= j; b++)
        {
          if ((j & *b) == *b)
            count++;
        }
        kCounts_[i][j] = count;
      }
    }
  }
}

/// Executed after each sieved segment.
/// @see sieveSegment() in SieveOfEratosthenes.cpp
///
void PrimeGenerator::generatePrimes(const byte_t* sieve, uint64_t sieveSize)
{
  if (ps_.isStore())
    storePrimes(ps_.getStore(), sieve, sieveSize);
  if (ps_.isCount())
    count(sieve, sieveSize);
  if (ps_.isPrint())
    print(sieve, sieveSize);
  if (ps_.isStatus())
    ps_.updateStatus(sieveSize * NUMBERS_PER_BYTE);
}

void PrimeGenerator::storePrimes(Store& store, const byte_t* sieve, uint64_t sieveSize) const
{
  uint64_t low = getSegmentLow();

  for (uint64_t i = 0; i < sieveSize; i += 8)
  {
    uint64_t bits = littleendian_cast<uint64_t>(&sieve[i]); 
    while (bits)
      store(nextPrime(&bits, low));

    low += NUMBERS_PER_BYTE * 8;
  }
}

/// Count the primes and prime k-tuplets
/// in the current segment
///
void PrimeGenerator::count(const byte_t* sieve, uint64_t sieveSize)
{
  if (ps_.isFlag(ps_.COUNT_PRIMES))
    counts_[0] += popcount((const uint64_t*) sieve, ceilDiv(sieveSize, 8));

  // i = 1 twins, i = 2 triplets, ...
  for (uint_t i = 1; i < counts_.size(); i++)
  {
    if (ps_.isCount(i))
    {
      uint64_t sum = 0;

      for (uint64_t j = 0; j < sieveSize; j += 4)
      {
        sum += kCounts_[i][sieve[j+0]];
        sum += kCounts_[i][sieve[j+1]];
        sum += kCounts_[i][sieve[j+2]];
        sum += kCounts_[i][sieve[j+3]];
      }

      counts_[i] += sum;
    }
  }
}

/// Print primes and prime k-tuplets to cout.
/// primes <= 5 are handled in processSmallPrimes().
///
void PrimeGenerator::print(const byte_t* sieve, uint64_t sieveSize) const
{
  if (ps_.isFlag(ps_.PRINT_PRIMES))
  {
    uint64_t low = getSegmentLow();

    for (uint64_t i = 0; i < sieveSize; i += 8)
    {
      uint64_t bits = littleendian_cast<uint64_t>(&sieve[i]); 
      while (bits)
        cout << nextPrime(&bits, low) << '\n';

      low += NUMBERS_PER_BYTE * 8;
    }
  }

  // print prime k-tuplets
  if (ps_.isFlag(ps_.PRINT_TWINS, ps_.PRINT_SEXTUPLETS))
  {
    uint_t i = 1; // i = 1 twins, i = 2 triplets, ...
    uint64_t low = getSegmentLow();

    for (; !ps_.isPrint(i); i++);
    for (uint64_t j = 0; j < sieveSize; j++, low += NUMBERS_PER_BYTE)
    {
      for (const uint64_t* bitmask = bitmasks_[i]; *bitmask <= sieve[j]; bitmask++)
      {
        if ((sieve[j] & *bitmask) == *bitmask)
        {
          ostringstream kTuplet;
          kTuplet << "(";
          uint64_t bits = *bitmask;
          while (bits != 0)
          {
            kTuplet << nextPrime(&bits, low);
            kTuplet << ((bits != 0) ? ", " : ")\n");
          }
          cout << kTuplet.str();
        }
      }
    }
  }
}

} // namespace
