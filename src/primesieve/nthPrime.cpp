///
/// @file  nthPrime.cpp
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/config.hpp>
#include <primesieve/PrimeSieve.hpp>
#include <primesieve/pmath.hpp>
#include <primesieve.hpp>

#include <stdint.h>
#include <algorithm>
#include <cmath>

using namespace std;
using namespace primesieve;

namespace {

void checkLimit(uint64_t start)
{
  if (start >= get_max_stop())
    throw primesieve_error("nth prime > 2^64");
}

void checkLowerLimit(uint64_t stop)
{
  if (stop == 0)
    throw primesieve_error("nth prime < 2 is impossible, n is too small");
}

// Prime count approximation
int64_t pix(int64_t n)
{
  double x = (double) n;
  double logx = log(max(4.0, x));
  double pix = x / logx;
  return (int64_t) pix;
}

uint64_t nthPrimeDist(int64_t n, int64_t count, uint64_t start)
{
  double x = (double) (n - count);

  x = abs(x);
  x = max(x, 4.0);

  double logx = log(x);
  double loglogx = log(logx);
  double pix = x * (logx + loglogx - 1);

  // correct start if sieving backwards to get
  // a more accurate approximation
  if (count >= n)
    start -= (uint64_t) pix;

  // approximate the nth prime using:
  // start + n * log(start + pi(n) / loglog(n))
  double startPix = start + pix / loglogx;
  startPix = max(4.0, startPix);
  double logStartPix = log(startPix);
  double dist = max(pix, x * logStartPix);

  // ensure (start + dist) <= nth prime
  if (count < n)
    dist -= sqrt(dist) * log(logStartPix) * 2;

  // ensure (start + dist) >= nth prime
  if (count > n)
    dist += sqrt(dist) * log(logStartPix) * 2;

  // ensure (start + dist) >= nth prime
  // if n is very small
  double maxPrimeGap = logStartPix * logStartPix;
  dist = max(dist, maxPrimeGap);
  return (uint64_t) dist;
}

bool sieveBackwards(int64_t n, int64_t count, uint64_t stop)
{
  return (count >= n) &&
        !(count == n && stop < 2);
}

} // namespace

namespace primesieve {

uint64_t PrimeSieve::nthPrime(uint64_t n)
{
  return nthPrime(0, n);
}

uint64_t PrimeSieve::nthPrime(int64_t n, uint64_t start)
{
  setStart(start);
  double t1 = getWallTime();

  if (n == 0)
    n = 1; // Like Mathematica
  else if (n > 0)
    start = add_overflow_safe(start, 1);
  else if (n < 0)
    start = sub_underflow_safe(start, 1);

  uint64_t stop = start;
  uint64_t dist = nthPrimeDist(n, 0, start);
  uint64_t nthPrimeGuess = add_overflow_safe(start, dist);

  int64_t count = 0;
  int64_t tinyN = 100000;
  tinyN = max(tinyN, pix(isqrt(nthPrimeGuess)));

  while ((n - count) > tinyN ||
         sieveBackwards(n, count, stop))
  {
    if (count < n)
    {
      checkLimit(start);
      dist = nthPrimeDist(n, count, start);
      stop = add_overflow_safe(start, dist);
      count += countPrimes(start, stop);
      start = add_overflow_safe(stop, 1);
    }
    if (sieveBackwards(n, count, stop))
    {
      checkLowerLimit(stop);
      dist = nthPrimeDist(n, count, stop);
      start = sub_underflow_safe(start, dist);
      count -= countPrimes(start, stop);
      stop = sub_underflow_safe(start, 1);
    }
  }

  try
  {
    checkLimit(start);
    if (n < 0) count -= 1;
    dist = nthPrimeDist(n, count, start) * 2;
    stop = add_overflow_safe(start, dist);
    uint64_t prime;
    for (primesieve::iterator it(start, stop); count < n; count++)
      prime = it.next_prime();
    seconds_ = getWallTime() - t1;
    return prime;
  }
  catch (primesieve_error&) { }

  throw primesieve_error("nth prime > 2^64");
}

} // namespace
