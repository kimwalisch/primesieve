///
/// @file   PrintPrimes.cpp
/// @brief  PrintPrimes is used for printing primes to stdout and
///         for counting primes. After a segment has been sieved
///         (using Erat) PrintPrimes is used to reconstruct primes
///         and prime k-tuplets from 1 bits of the sieve array.
///
/// Copyright (C) 2019 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/littleendian_cast.hpp>
#include <primesieve/pmath.hpp>
#include <primesieve/PrintPrimes.hpp>
#include <primesieve/PrimeSieve.hpp>
#include <primesieve/Erat.hpp>
#include <primesieve/SievingPrimes.hpp>
#include <primesieve/types.hpp>

#include <stdint.h>
#include <algorithm>
#include <iostream>
#include <sstream>

using namespace std;

namespace {

const uint64_t bitmasks[6][5] =
{
  { ~0ull },                         // Prime numbers, unused
  { 0x06, 0x18, 0xc0, ~0ull },       // Twin primes: b00000110, b00011000, b11000000
  { 0x07, 0x0e, 0x1c, 0x38, ~0ull }, // Prime triplets: b00000111, b00001110, ...
  { 0x1e, ~0ull },                   // Prime quadruplets: b00011110
  { 0x1f, 0x3e, ~0ull },             // Prime quintuplets: b00011111, b00111110
  { 0x3f, ~0ull }                    // Prime sextuplets: b00111111
};

} // namespace

namespace primesieve {

PrintPrimes::PrintPrimes(PrimeSieve& ps) :
  counts_(ps.getCounts()),
  ps_(ps)
{
  uint64_t start = ps.getStart();
  uint64_t stop = ps.getStop();
  uint64_t sieveSize = ps.getSieveSize();
  start = max<uint64_t>(start, 7);

  Erat::init(start, stop, sieveSize, ps.getPreSieve());

  if (ps_.isCountkTuplets())
    initCounts();
}

/// Initialize the lookup tables to count the number
/// of twins, triplets, ... per byte
///
void PrintPrimes::initCounts()
{
  for (uint_t i = 1; i < counts_.size(); i++)
  {
    if (!ps_.isCount(i))
      continue;

    kCounts_[i].resize(256);

    for (uint64_t j = 0; j < 256; j++)
    {
      byte_t count = 0;
      for (const uint64_t* b = bitmasks[i]; *b <= j; b++)
      {
        if ((j & *b) == *b)
          count++;
      }
      kCounts_[i][j] = count;
    }
  }
}

void PrintPrimes::sieve()
{
  SievingPrimes sievingPrimes(this, ps_.getPreSieve());
  uint64_t prime = sievingPrimes.next();

  while (hasNextSegment())
  {
    low_ = segmentLow_;
    uint64_t sqrtHigh = isqrt(segmentHigh_);

    for (; prime <= sqrtHigh; prime = sievingPrimes.next())
      addSievingPrime(prime);

    sieveSegment();
    print();
  }
}

/// Executed after each sieved segment
void PrintPrimes::print()
{
  if (ps_.isCountPrimes())
    countPrimes();
  if (ps_.isCountkTuplets())
    countkTuplets();
  if (ps_.isPrintPrimes())
    printPrimes();
  if (ps_.isPrintkTuplets())
    printkTuplets();
  if (ps_.isStatus())
    ps_.updateStatus(sieveSize_ * 30);
}

void PrintPrimes::countPrimes()
{
  uint64_t size = ceilDiv(sieveSize_, 8);
  counts_[0] += popcount((const uint64_t*) sieve_, size);
}

void PrintPrimes::countkTuplets()
{
  // i = 1 twins, i = 2 triplets, ...
  for (uint_t i = 1; i < counts_.size(); i++)
  {
    if (!ps_.isCount(i))
      continue;

    uint64_t sum = 0;

    for (uint64_t j = 0; j < sieveSize_; j += 4)
    {
      sum += kCounts_[i][sieve_[j+0]];
      sum += kCounts_[i][sieve_[j+1]];
      sum += kCounts_[i][sieve_[j+2]];
      sum += kCounts_[i][sieve_[j+3]];
    }

    counts_[i] += sum;
  }
}

/// Print primes to stdout
void PrintPrimes::printPrimes() const
{
  uint64_t i = 0;
  uint64_t low = low_;

  while (i < sieveSize_)
  {
    uint64_t size = min(i + (1 << 16), sieveSize_);
    ostringstream primes;

    for (; i < size; i += 8)
    {
      uint64_t bits = littleendian_cast<uint64_t>(&sieve_[i]);
      while (bits)
        primes << nextPrime(&bits, low) << '\n';

      low += 8 * 30;
    }

    cout << primes.str();
  }
}

/// Print prime k-tuplets to stdout
void PrintPrimes::printkTuplets() const
{
  ostringstream kTuplets;
  // i = 1 twins, i = 2 triplets, ...
  uint_t i = 1;
  uint64_t low = low_;

  for (; !ps_.isPrint(i); i++);

  for (uint64_t j = 0; j < sieveSize_; j++, low += 30)
  {
    for (const uint64_t* bitmask = bitmasks[i]; *bitmask <= sieve_[j]; bitmask++)
    {
      if ((sieve_[j] & *bitmask) == *bitmask)
      {
        kTuplets << "(";
        uint64_t bits = *bitmask;
        while (bits != 0)
        {
          kTuplets << nextPrime(&bits, low);
          kTuplets << ((bits != 0) ? ", " : ")\n");
        }
      }
    }
  }

  cout << kTuplets.str();
}

} // namespace
