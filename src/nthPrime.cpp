///
/// @file  nthPrime.cpp
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/iterator.hpp>
#include <primesieve/forward.hpp>
#include <primesieve/PrimeSieve.hpp>
#include <primesieve/pmath.hpp>
#include <primesieve/primesieve_error.hpp>

#include <stdint.h>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>

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
    throw primesieve_error("nth prime < 2 is impossible");
}

bool sieveBackwards(int64_t n, int64_t count, uint64_t stop)
{
  return (count >= n) &&
        !(count == n && stop < 2);
}

// Prime count approximation
int64_t pix(int64_t n)
{
  double x = (double) n;
  x = std::max(4.0, x);
  double pix = x / std::log(x);
  return (int64_t) pix;
}

uint64_t nthPrimeDist(int64_t n, int64_t count, uint64_t start)
{
  double x = (double) (n - count);

  x = std::abs(x);
  x = std::max(x, 4.0);

  // rough pi(x) approximation
  double logx = std::log(x);
  double loglogx = std::log(logx);
  double pix = x * (logx + loglogx - 1);

  // correct start if sieving backwards to
  // get more accurate approximation
  if (count >= n)
  {
    double st = start - pix;
    st = std::max(0.0, st);
    start = (uint64_t) st;
  }

  // approximate the nth prime using:
  // start + n * log(start + pi(n) / loglog(n))
  double startPix = start + pix / loglogx;
  startPix = std::max(4.0, startPix);
  double logStartPix = std::log(startPix);
  double dist = std::max(pix, x * logStartPix);

  // ensure start + dist <= nth prime
  if (count < n)
    dist -= std::sqrt(dist) * std::log(logStartPix) * 2;
  // ensure start + dist >= nth prime
  if (count > n)
    dist += std::sqrt(dist) * std::log(logStartPix) * 2;

  // if n is very small:
  // ensure start + dist >= nth prime
  double primeGap = maxPrimeGap(startPix);
  dist = std::max(dist, primeGap);

  return (uint64_t) dist;
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
  auto t1 = std::chrono::system_clock::now();

  if (n == 0)
    n = 1; // like Mathematica
  else if (n > 0)
    start = checkedAdd(start, 1);
  else if (n < 0)
    start = checkedSub(start, 1);

  uint64_t stop = start;
  uint64_t dist = nthPrimeDist(n, 0, start);
  uint64_t nthPrimeGuess = checkedAdd(start, dist);

  int64_t count = 0;
  int64_t tinyN = 100000;
  tinyN = std::max(tinyN, pix(isqrt(nthPrimeGuess)));

  while ((n - count) > tinyN ||
         sieveBackwards(n, count, stop))
  {
    if (count < n)
    {
      checkLimit(start);
      dist = nthPrimeDist(n, count, start);
      stop = checkedAdd(start, dist);
      count += countPrimes(start, stop);
      start = checkedAdd(stop, 1);
    }
    if (sieveBackwards(n, count, stop))
    {
      checkLowerLimit(stop);
      dist = nthPrimeDist(n, count, stop);
      start = checkedSub(start, dist);
      count -= (int64_t) countPrimes(start, stop);
      stop = checkedSub(start, 1);
    }
  }

  if (n < 0)
    count -= 1;

  // here start < nth prime,
  // hence we can sieve forward the remaining
  // distance and find the nth prime
  assert(count < n);

  checkLimit(start);
  dist = nthPrimeDist(n, count, start) * 2;
  start = checkedSub(start, 1);
  stop = checkedAdd(start, dist);
  uint64_t prime = 0;

  for (primesieve::iterator it(start, stop); count < n; count++)
    prime = it.next_prime();

  if (~prime == 0)
    throw primesieve_error("nth prime > 2^64");

  auto t2 = std::chrono::system_clock::now();
  std::chrono::duration<double> seconds = t2 - t1;
  seconds_ = seconds.count();

  return prime;
}

} // namespace
