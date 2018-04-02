///
/// @file   PrimeGenerator.cpp
/// @brief  After a segment has been sieved PrimeGenerator is
///         used to reconstruct primes and prime k-tuplets from
///         1 bits of the sieve array.
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/config.hpp>
#include <primesieve/littleendian_cast.hpp>
#include <primesieve/pmath.hpp>
#include <primesieve/PreSieve.hpp>
#include <primesieve/PrimeGenerator.hpp>
#include <primesieve/PrimeSieve.hpp>
#include <primesieve/Erat.hpp>
#include <primesieve/SievingPrimes.hpp>
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

PrimeGenerator::PrimeGenerator(PrimeSieve& ps) :
  preSieve_(ps.getStart(), ps.getStop()),
  counts_(ps.getCounts()),
  ps_(ps)
{
  uint64_t start = max<uint64_t>(7, ps.getStart());
  uint64_t stop = ps.getStop();
  uint64_t sieveSize = ps.getSieveSize();

  Erat::init(start, stop, sieveSize, preSieve_);

  if (ps_.isFlag(ps_.COUNT_TWINS, ps_.COUNT_SEXTUPLETS))
    initCounts();
}

/// Initialize the lookup tables to count the number
/// of twins, triplets, ... per byte
///
void PrimeGenerator::initCounts()
{
  for (uint_t i = 1; i < counts_.size(); i++)
  {
    if (!ps_.isCount(i))
      continue;

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

void PrimeGenerator::sieve()
{
  if (sqrtStop_ > preSieve_.getMaxPrime())
  {
    // generate sieving primes
    SievingPrimes sievingPrimes(*this, preSieve_);
    uint64_t prime = sievingPrimes.nextPrime();
    for (; prime <= sqrtStop_; prime = sievingPrimes.nextPrime())
      Erat::sieve(prime);
  }

  Erat::sieve();
}

/// Executed after each sieved segment
void PrimeGenerator::generatePrimes(const byte_t* sieve, uint64_t sieveSize)
{
  if (ps_.isStore())
    storePrimes(ps_.getStore(), sieve, sieveSize);
  if (ps_.isFlag(ps_.COUNT_PRIMES))
    countPrimes(sieve, sieveSize);
  if (ps_.isFlag(ps_.COUNT_TWINS, ps_.COUNT_SEXTUPLETS))
    countkTuplets(sieve, sieveSize);
  if (ps_.isFlag(ps_.PRINT_PRIMES))
    printPrimes(sieve, sieveSize);
  if (ps_.isFlag(ps_.PRINT_TWINS, ps_.PRINT_SEXTUPLETS))
    printkTuplets(sieve, sieveSize);
  if (ps_.isStatus())
    ps_.updateStatus(sieveSize * 30);
}

void PrimeGenerator::storePrimes(Store& store, const byte_t* sieve, uint64_t sieveSize) const
{
  uint64_t low = segmentLow_;

  for (uint64_t i = 0; i < sieveSize; i += 8)
  {
    uint64_t bits = littleendian_cast<uint64_t>(&sieve[i]); 
    while (bits)
      store(getPrime(&bits, low));

    low += 30 * 8;
  }
}

void PrimeGenerator::countPrimes(const byte_t* sieve, uint64_t sieveSize)
{
  uint64_t size = ceilDiv(sieveSize, 8);
  counts_[0] += popcount((const uint64_t*) sieve, size);
}

void PrimeGenerator::countkTuplets(const byte_t* sieve, uint64_t sieveSize)
{
  // i = 1 twins, i = 2 triplets, ...
  for (uint_t i = 1; i < counts_.size(); i++)
  {
    if (!ps_.isCount(i))
      continue;

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

/// Print primes to stdout
void PrimeGenerator::printPrimes(const byte_t* sieve, uint64_t sieveSize) const
{
  uint64_t i = 0;
  uint64_t low = segmentLow_;

  while (i < sieveSize)
  {
    uint64_t size = min(i + (1 << 16), sieveSize);
    ostringstream primes;

    for (; i < size; i += 8)
    {
      uint64_t bits = littleendian_cast<uint64_t>(&sieve[i]);
      while (bits)
        primes << getPrime(&bits, low) << '\n';

      low += 30 * 8;
    }

    cout << primes.str();
  }
}

/// Print prime k-tuplets to stdout
void PrimeGenerator::printkTuplets(const byte_t* sieve, uint64_t sieveSize) const
{
  // i = 1 twins, i = 2 triplets, ...
  uint_t i = 1;
  uint64_t low = segmentLow_;
  ostringstream kTuplets;

  for (; !ps_.isPrint(i); i++);

  for (uint64_t j = 0; j < sieveSize; j++, low += 30)
  {
    for (const uint64_t* bitmask = bitmasks_[i]; *bitmask <= sieve[j]; bitmask++)
    {
      if ((sieve[j] & *bitmask) == *bitmask)
      {
        kTuplets << "(";
        uint64_t bits = *bitmask;
        while (bits != 0)
        {
          kTuplets << getPrime(&bits, low);
          kTuplets << ((bits != 0) ? ", " : ")\n");
        }
      }
    }
  }

  cout << kTuplets.str();
}

} // namespace
