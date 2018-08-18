///
/// @file  PrimeGenerator.cpp
///        Generates the primes inside [start, stop] and stores them
///        in a vector. After the primes have been stored in the
///        vector primesieve::iterator iterates over the vector and
///        returns the primes. When there are no more primes left in
///        the vector PrimeGenerator generates new primes.
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/Erat.hpp>
#include <primesieve/PreSieve.hpp>
#include <primesieve/PrimeGenerator.hpp>
#include <primesieve/pmath.hpp>
#include <primesieve/SievingPrimes.hpp>
#include <primesieve/types.hpp>

#include <stdint.h>
#include <algorithm>
#include <array>
#include <vector>

using namespace std;

namespace primesieve {

/// First 64 primes
const array<uint64_t, 64> PrimeGenerator::smallPrimes =
{
    2,   3,   5,   7,  11,  13,  17,  19,
   23,  29,  31,  37,  41,  43,  47,  53,
   59,  61,  67,  71,  73,  79,  83,  89,
   97, 101, 103, 107, 109, 113, 127, 131,
  137, 139, 149, 151, 157, 163, 167, 173,
  179, 181, 191, 193, 197, 199, 211, 223,
  227, 229, 233, 239, 241, 251, 257, 263,
  269, 271, 277, 281, 283, 293, 307, 311
};

/// Number of primes <= n
const array<uint8_t, 312> PrimeGenerator::primePi =
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
  52, 53, 53, 53, 53, 53, 53, 53, 53, 53,
  53, 54, 54, 54, 54, 54, 54, 55, 55, 55,
  55, 55, 55, 56, 56, 56, 56, 56, 56, 57,
  57, 58, 58, 58, 58, 58, 58, 59, 59, 59,
  59, 60, 60, 61, 61, 61, 61, 61, 61, 61,
  61, 61, 61, 62, 62, 62, 62, 62, 62, 62,
  62, 62, 62, 62, 62, 62, 62, 63, 63, 63,
  63, 64
};

PrimeGenerator::PrimeGenerator(uint64_t start, uint64_t stop) :
  Erat(start, stop)
{ }

void PrimeGenerator::init(vector<uint64_t>& primes)
{
  size_t size = primeCountApprox(start_, stop_);
  primes.reserve(size);

  if (start_ <= maxCachedPrime())
  {
    size_t a = getStartIdx();
    size_t b = getStopIdx();

    primes.insert(primes.end(),
             smallPrimes.begin() + a,
             smallPrimes.begin() + b);
  }

  initErat();
}

void PrimeGenerator::init(vector<uint64_t>& primes, size_t* size)
{
  if (start_ <= maxCachedPrime())
  {
    size_t a = getStartIdx();
    size_t b = getStopIdx();
    *size = b - a;

    copy(smallPrimes.begin() + a,
         smallPrimes.begin() + b,
         primes.begin());
  }

  initErat();
}

void PrimeGenerator::initErat()
{
  uint64_t startErat = maxCachedPrime() + 1;
  startErat = max(startErat, start_);
  isInit_ = true;

  if (startErat <= stop_)
  {
    int sieveSize = get_sieve_size();
    Erat::init(startErat, stop_, sieveSize, preSieve_);
    sievingPrimes_.init(this, preSieve_);
  }
}

size_t PrimeGenerator::getStartIdx() const
{
  size_t startIdx = 0;

  if (start_ > 1)
    startIdx = primePi[start_ - 1];

  return startIdx;
}

size_t PrimeGenerator::getStopIdx() const
{
  size_t stopIdx = 0;

  if (stop_ < maxCachedPrime())
    stopIdx = primePi[stop_];
  else
    stopIdx = smallPrimes.size();

  return stopIdx;
}

bool PrimeGenerator::sieveSegment(vector<uint64_t>& primes)
{
  if (!isInit_)
    init(primes);

  if (!hasNextSegment())
    return false;

  sieveSegment();
  return true;
}

bool PrimeGenerator::sieveSegment(vector<uint64_t>& primes, size_t* size)
{
  if (!isInit_)
  {
    init(primes, size);
    if (*size > 0)
      return false;
  }

  if (!hasNextSegment())
  {
    *size = 1;
    primes[0] = ~0ull;
    finished_ = (primes[0] > stop_);
    return false;
  }

  sieveSegment();
  return true;
}

void PrimeGenerator::sieveSegment()
{
  uint64_t sqrtHigh = isqrt(segmentHigh_);

  sieveIdx_ = 0;
  low_ = segmentLow_;

  if (!prime_)
    prime_ = sievingPrimes_.next();

  while (prime_ <= sqrtHigh)
  {
    addSievingPrime(prime_);
    prime_ = sievingPrimes_.next();
  }

  Erat::sieveSegment();
}

void PrimeGenerator::fill(vector<uint64_t>& primes)
{
  while (sieveSegment(primes))
  {
    for (; sieveIdx_ < sieveSize_; sieveIdx_ += 8)
    {
      uint64_t bits = littleendian_cast<uint64_t>(&sieve_[sieveIdx_]);

      while (bits)
        primes.push_back(nextPrime(&bits, low_));

      low_ += 8 * 30;
    }
  }
}

} // namespace
