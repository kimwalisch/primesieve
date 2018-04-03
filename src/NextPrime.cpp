///
/// @file  NextPrime.cpp
///        This class implements a nextPrime() method
///        for iterating over primes.
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/NextPrime.hpp>
#include <primesieve/PreSieve.hpp>
#include <primesieve/Erat.hpp>
#include <primesieve/littleendian_cast.hpp>
#include <primesieve/SievingPrimes.hpp>

#include <stdint.h>
#include <algorithm>
#include <array>

using namespace std;

namespace {

/// First 53 primes
const std::array<uint64_t, 53> smallPrimes =
{
    2,   3,   5,   7,  11,  13,  17,  19,
   23,  29,  31,  37,  41,  43,  47,  53,
   59,  61,  67,  71,  73,  79,  83,  89,
   97, 101, 103, 107, 109, 113, 127, 131,
  137, 139, 149, 151, 157, 163, 167, 173,
  179, 181, 191, 193, 197, 199, 211, 223,
  227, 229, 233, 239, 241
};

/// Number of primes <= n
const std::array<size_t, 242> primePi =
{
   0,  0,  1,  2,  2,  3,  3,  4,  4,  4,
   4,  5,  5,  6,  6,  6,  6,  7,  7,  8,
   8,  8,  8,  9,  9,  9,  9,  9,  9, 10,
  10, 11, 11, 11, 11, 11, 11, 12, 12, 12,
  12, 13, 13, 14, 14, 14, 14, 15, 15, 15,
  15, 15, 15, 16, 16, 16, 16, 16, 16, 17,
  17, 18, 18, 18, 18, 18, 18, 19, 19, 19,
  19, 20, 20, 21, 21, 21, 21, 21, 21, 22,
  22, 22, 22, 23, 23, 23, 23, 23, 23, 24,
  24, 24, 24, 24, 24, 24, 24, 25, 25, 25,
  25, 26, 26, 27, 27, 27, 27, 28, 28, 29,
  29, 29, 29, 30, 30, 30, 30, 30, 30, 30,
  30, 30, 30, 30, 30, 30, 30, 31, 31, 31,
  31, 32, 32, 32, 32, 32, 32, 33, 33, 34,
  34, 34, 34, 34, 34, 34, 34, 34, 34, 35,
  35, 36, 36, 36, 36, 36, 36, 37, 37, 37,
  37, 37, 37, 38, 38, 38, 38, 39, 39, 39,
  39, 39, 39, 40, 40, 40, 40, 40, 40, 41,
  41, 42, 42, 42, 42, 42, 42, 42, 42, 42,
  42, 43, 43, 44, 44, 44, 44, 45, 45, 46,
  46, 46, 46, 46, 46, 46, 46, 46, 46, 46,
  46, 47, 47, 47, 47, 47, 47, 47, 47, 47,
  47, 47, 47, 48, 48, 48, 48, 49, 49, 50,
  50, 50, 50, 51, 51, 51, 51, 51, 51, 52,
  52, 53
};

} // namespace

namespace primesieve {

NextPrime::NextPrime(uint64_t start, uint64_t stop, uint64_t sieveSize)
  : preSieve_(start, stop)
{
  initSmallPrimes(start, stop);

  // small primes < 247 are stored in lookup table,
  // sieving is only used if stop >= 247
  uint64_t minStart = 247;
  start = max(start, minStart);
  stop = max(start, stop);

  Erat::init(start, stop, sieveSize, preSieve_);
  sievingPrimes_.init(this, preSieve_);
  low_ = segmentLow_;
}

void NextPrime::initSmallPrimes(uint64_t start, uint64_t stop)
{
  if (start <= smallPrimes.back())
  {
    size_t startIdx = 0;
    size_t stopIdx = smallPrimes.size();

    if (start > 1)
      startIdx = primePi[start] - 1;
    if (stop < smallPrimes.back())
      stopIdx = primePi[stop];

    copy(&smallPrimes[startIdx], &smallPrimes[stopIdx], primes_);
    num_ = stopIdx - startIdx;
  }
}

void NextPrime::fill()
{
  i_ = 0;

  if (sieveIdx_ >= sieveSize_)
    if (!sieveSegment())
      return;

  uint64_t bits = littleendian_cast<uint64_t>(&sieve_[sieveIdx_]);
  sieveIdx_ += 8;
  uint64_t num = 0;

  for (; bits != 0; num++)
    primes_[num] = getPrime(&bits, low_);

  num_ = num;
  low_ += 30 * 8;
}

bool NextPrime::sieveSegment()
{
  if (hasNextSegment())
  {
    sieveIdx_ = 0;
    uint64_t high = min(segmentHigh_, stop_);
    uint64_t sqrtHigh = isqrt(high);

    if (!sievingPrime_)
      sievingPrime_ = sievingPrimes_.nextPrime();

    while (sievingPrime_ <= sqrtHigh)
    {
      addSievingPrime(sievingPrime_);
      sievingPrime_ = sievingPrimes_.nextPrime();
    }

    Erat::sieveSegment();
    return true;
  }
  else
  {
    num_ = 1;
    primes_[0] = ~0ull;
    return false;
  }
}

} // namespace
