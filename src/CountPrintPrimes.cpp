///
/// @file   CountPrintPrimes.cpp
/// @brief  CountPrintPrimes is used for counting primes and for
///         printing primes to stdout. After a segment has been sieved
///         (using the parent Erat class) CountPrintPrimes is used
///         to reconstruct primes and prime k-tuplets from 1 bits of
///         the sieve array.
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include "CountPrintPrimes.hpp"
#include "Erat.hpp"
#include "PrimeSieveClass.hpp"
#include "SievingPrimes.hpp"

#include <primesieve/forward.hpp>
#include <primesieve/littleendian_cast.hpp>
#include <primesieve/macros.hpp>
#include <primesieve/pmath.hpp>

#include <stdint.h>
#include <algorithm>
#include <iostream>

using namespace primesieve;

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

#if __cplusplus >= 201703L && \
    __has_include(<charconv>)

#include <primesieve.hpp>
#include <charconv>

namespace {

/// Appends a uint64_t prime to the char vector as a decimal
/// string. This implementation uses std::to_chars() for
/// fast, zero-allocation and zero-copy conversion directly
/// into the vector's pre-reserved buffer memory.
///
ALWAYS_INLINE void append_prime_as_string(Vector<char>& vect, uint64_t prime)
{
  std::size_t old_size = vect.size();

  // Converting a 64-bit integer to a string
  // requires at most 20 characters.
  vect.resize(old_size + 20);
  char* first = &vect[old_size];
  char* last = vect.end();

  std::to_chars_result res = std::to_chars(first, last, prime);

  if (res.ec == std::errc{})
    vect.resize(vect.size() - (last - res.ptr));
  else
    throw primesieve_error("append_prime_as_string(): failed to convert prime to string!");
}

} // namespace

#else

#include <string>

namespace {

ALWAYS_INLINE void append_prime_as_string(Vector<char>& vect, uint64_t prime)
{
  std::string str = std::to_string(prime);
  vect.insert(vect.end(), str.begin(), str.end());
}

} // namespace

#endif

namespace primesieve {

CountPrintPrimes::CountPrintPrimes(PrimeSieve& ps) :
  counts_(ps.getCounts()),
  ps_(ps)
{
  uint64_t start = ps.getStart();
  uint64_t stop = ps.getStop();
  uint64_t sieveSize = ps.getSieveSize();
  start = std::max<uint64_t>(start, 7);

  Erat::init(start, stop, sieveSize, memoryPool_);

  if (ps_.isCountkTuplets())
    initCounts();
}

/// Initialize the lookup tables to count the number
/// of twins, triplets, ... per byte
///
void CountPrintPrimes::initCounts()
{
  for (unsigned i = 1; i < counts_.size(); i++)
  {
    if (ps_.isCount(i))
    {
      kCounts_[i].resize(256);

      for (uint64_t j = 0; j < 256; j++)
      {
        kCounts_[i][j] = 0;
        for (auto* b = bitmasks[i]; *b <= j; b++)
          if ((j & *b) == *b)
            kCounts_[i][j]++;
      }
    }
  }
}

void CountPrintPrimes::sieve()
{
  uint64_t sieveSize = ps_.getSieveSize();
  SievingPrimes sievingPrimes(this, sieveSize, memoryPool_);
  uint64_t prime = sievingPrimes.next();

  while (hasNextSegment())
  {
    low_ = segmentLow_;
    uint64_t sqrtHigh = isqrt(segmentHigh_);

    for (; prime <= sqrtHigh; prime = sievingPrimes.next())
      addSievingPrime(prime);

    sieveSegment();

    if (ps_.isCountPrimes())
      countPrimes();
    if (ps_.isCountkTuplets())
      countkTuplets();
    if (ps_.isPrintPrimes())
      printPrimes();
    if (ps_.isPrintkTuplets())
      printkTuplets();
    if (ps_.isStatus())
      ps_.updateStatus(sieve_.size() * 30);
  }
}

void CountPrintPrimes::countPrimes()
{
  ASSERT(sieve_.capacity() % sizeof(uint64_t) == 0);
  uint64_t size = ceilDiv(sieve_.size(), 8);
  counts_[0] += popcount((const uint64_t*) sieve_.data(), size);
}

void CountPrintPrimes::countkTuplets()
{
  // i = 1 twins, i = 2 triplets, ...
  for (unsigned i = 1; i < counts_.size(); i++)
  {
    if (ps_.isCount(i))
    {
      ASSERT(sieve_.capacity() % 4 == 0);
      auto* sieve = sieve_.data();
      uint64_t sum = 0;

      for (std::size_t j = 0; j < sieve_.size(); j += 4)
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

/// Print primes to stdout
void CountPrintPrimes::printPrimes()
{
  uint64_t low = low_;
  std::size_t i = 0;

  while (i < sieve_.size())
  {
    charBuffer_.clear();
    std::size_t size = i + (1 << 16);
    size = std::min(size, sieve_.size());

    for (; i < size; i += 8)
    {
      uint64_t bits = littleendian_cast<uint64_t>(&sieve_[i]);

      for (; bits != 0; bits &= bits - 1)
      {
        uint64_t prime = nextPrime(bits, low);
        append_prime_as_string(charBuffer_, prime);
        charBuffer_.push_back('\n');
      }

      low += 8 * 30;
    }

    std::cout.write(charBuffer_.data(), charBuffer_.size());
  }
}

/// Print prime k-tuplets to stdout
void CountPrintPrimes::printkTuplets()
{
  // i = 1 twins, i = 2 triplets, ...
  unsigned i = 1;
  uint64_t low = low_;
  charBuffer_.clear();

  while (!ps_.isPrint(i))
    i++;

  for (std::size_t j = 0; j < sieve_.size(); j++, low += 30)
  {
    for (auto* bitmask = bitmasks[i]; *bitmask <= sieve_[j]; bitmask++)
    {
      if ((sieve_[j] & *bitmask) == *bitmask)
      {
        charBuffer_.push_back('(');
        uint64_t bits = *bitmask;

        for (; bits != 0; bits &= bits - 1)
        {
          uint64_t prime = nextPrime(bits, low);
          append_prime_as_string(charBuffer_, prime);

          if (bits & (bits - 1))
          {
            charBuffer_.push_back(',');
            charBuffer_.push_back(' ');
          }
          else
          {
            charBuffer_.push_back(')');
            charBuffer_.push_back('\n');
          }
        }
      }
    }
  }

  std::cout.write(charBuffer_.data(), charBuffer_.size());
}

} // namespace
