///
/// @file   PrimeFinder.cpp
/// @brief  Callback, print and count primes and prime k-tuplets
///         (twin primes, prime triplets, ...).
///
/// Copyright (C) 2016 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/config.hpp>
#include <primesieve/PrimeFinder.hpp>
#include <primesieve/PrimeSieve.hpp>
#include <primesieve/PrimeSieve-lock.hpp>
#include <primesieve/Callback.hpp>
#include <primesieve/callback_t.hpp>
#include <primesieve/SieveOfEratosthenes.hpp>
#include <primesieve/SieveOfEratosthenes-inline.hpp>
#include <primesieve/littleendian_cast.hpp>

#include <stdint.h>
#include <algorithm>
#include <vector>
#include <iostream>
#include <sstream>

namespace primesieve {

class PreSieve;

uint64_t popcount(const uint64_t* array, uint64_t size);

const uint_t PrimeFinder::kBitmasks_[6][5] =
{
  { END },
  { 0x06, 0x18, 0xc0, END },       // Twin prime       bitmasks, i.e. b00000110, b00011000, b11000000
  { 0x07, 0x0e, 0x1c, 0x38, END }, // Prime triplet    bitmasks, i.e. b00000111, b00001110, ...
  { 0x1e, END },                   // Prime quadruplet bitmasks
  { 0x1f, 0x3e, END },             // Prime quintuplet bitmasks
  { 0x3f, END }                    // Prime sextuplet  bitmasks
};

PrimeFinder::PrimeFinder(PrimeSieve& ps, const PreSieve& preSieve) :
  SieveOfEratosthenes(std::max<uint64_t>(7, ps.getStart()),
                      ps.getStop(),
                      ps.getSieveSize(),
                      preSieve),
  ps_(ps)
{
  if (ps_.isFlag(ps_.COUNT_TWINS, ps_.COUNT_SEXTUPLETS))
    init_kCounts();
}

/// Calculate the number of twins, triplets, ... (bitmask matches)
/// for each possible byte value 0 - 255.
///
void PrimeFinder::init_kCounts()
{
  for (uint_t i = 1; i < ps_.counts_.size(); i++)
  {
    if (ps_.isCount(i))
    {
      kCounts_[i].resize(256);
      for (uint_t j = 0; j < kCounts_[i].size(); j++)
      {
        uint_t bitmaskCount = 0;
        for (const uint_t* b = kBitmasks_[i]; *b <= j; b++)
        {
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
void PrimeFinder::segmentFinished(const byte_t* sieve, uint_t sieveSize)
{
  if (ps_.isCallback())
    callbackPrimes(sieve, sieveSize);
  if (ps_.isCount())
    count(sieve, sieveSize);
  if (ps_.isPrint())
    print(sieve, sieveSize);
  if (ps_.isStatus())
    ps_.updateStatus(sieveSize * NUMBERS_PER_BYTE);
}

/// Reconstruct prime numbers from 1 bits of the sieve array and
/// call a callback function for each prime.
///
template <typename T>
inline void PrimeFinder::callbackPrimes(T callback, const byte_t* sieve, uint_t sieveSize) const
{
  uint64_t base = getSegmentLow();
  for (uint_t i = 0; i < sieveSize; i += 8, base += NUMBERS_PER_BYTE * 8)
  {
    uint64_t bits = littleendian_cast<uint64_t>(&sieve[i]); 
    while (bits != 0)
    {
      uint64_t prime = getNextPrime(&bits, base);
      callback(prime);
    }
  }
}

template <>
inline void PrimeFinder::callbackPrimes(Callback<uint64_t>* cb, const byte_t* sieve, uint_t sieveSize) const
{
  uint64_t base = getSegmentLow();
  for (uint_t i = 0; i < sieveSize; i += 8, base += NUMBERS_PER_BYTE * 8)
  {
    uint64_t bits = littleendian_cast<uint64_t>(&sieve[i]); 
    while (bits != 0)
    {
      uint64_t prime = getNextPrime(&bits, base);
      cb->callback(prime);
    }
  }
}

/// Callback the primes within the current segment.
/// @note primes < 7 are handled in PrimeSieve::doSmallPrime()
///
void PrimeFinder::callbackPrimes(const byte_t* sieve, uint_t sieveSize) const
{
  if (ps_.isFlag(ps_.CALLBACK_PRIMES_OBJ)) callbackPrimes(ps_.cb_, sieve, sieveSize);
  if (ps_.isFlag(ps_.CALLBACK_PRIMES))     callbackPrimes(ps_.callback_, sieve, sieveSize);
  if (ps_.isFlag(ps_.CALLBACK_PRIMES_C))   callbackPrimes(reinterpret_cast<callback_c_t>(ps_.callback_), sieve, sieveSize);
}

/// Count the primes and prime k-tuplets within
/// the current segment.
///
void PrimeFinder::count(const byte_t* sieve, uint_t sieveSize)
{
  // count prime numbers (1 bits), see popcount.cpp
  if (ps_.isFlag(ps_.COUNT_PRIMES))
    ps_.counts_[0] += popcount(reinterpret_cast<const uint64_t*>(sieve), ceilDiv(sieveSize, 8));

  // count prime k-tuplets (i = 1 twins, i = 2 triplets, ...)
  for (uint_t i = 1; i < ps_.counts_.size(); i++)
  {
    if (ps_.isCount(i))
    {
      uint_t sum0 = 0;
      uint_t sum1 = 0;
      uint_t sum2 = 0;
      uint_t sum3 = 0;
      for (uint_t j = 0; j < sieveSize; j += 4)
      {
        sum0 += kCounts_[i][sieve[j+0]];
        sum1 += kCounts_[i][sieve[j+1]];
        sum2 += kCounts_[i][sieve[j+2]];
        sum3 += kCounts_[i][sieve[j+3]];
      }
      ps_.counts_[i] += (sum0 + sum1) + (sum2 + sum3);
    }
  }
}

void PrimeFinder::printPrime(uint64_t prime)
{
  std::cout << prime << '\n';
}

/// Print primes and prime k-tuplets to cout.
/// @note primes < 7 are handled in PrimeSieve::doSmallPrime()
///
void PrimeFinder::print(const byte_t* sieve, uint_t sieveSize) const
{
  if (ps_.isFlag(ps_.PRINT_PRIMES))
  {
    LockGuard lock(ps_);
    callbackPrimes(printPrime, sieve, sieveSize);
  }
  // print prime k-tuplets
  if (ps_.isFlag(ps_.PRINT_TWINS, ps_.PRINT_SEXTUPLETS))
  {
    uint_t i = 1; // i = 1 twins, i = 2 triplets, ...
    uint64_t base = getSegmentLow();

    for (; !ps_.isPrint(i); i++);
    for (uint_t j = 0; j < sieveSize; j++, base += NUMBERS_PER_BYTE)
    {
      for (const uint_t* bitmask = kBitmasks_[i]; *bitmask <= sieve[j]; bitmask++)
      {
        if ((sieve[j] & *bitmask) == *bitmask)
        {
          std::ostringstream kTuplet;
          kTuplet << "(";
          uint64_t bits = *bitmask;
          while (bits != 0)
          {
            kTuplet << getNextPrime(&bits, base);
            kTuplet << ((bits != 0) ? ", " : ")\n");
          }
          std::cout << kTuplet.str();
        }
      }
    }
  }
}

} // namespace
