///
/// @file  PrimeSieve-nthPrime.cpp
///
/// Copyright (C) 2016 Kim Walisch, <kim.walisch@gmail.com>
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
    throw primesieve_error("nth prime < 2 is impossible, negative n is too small");
}

void checkLowerLimit(uint64_t start, double dist)
{
  double s = static_cast<double>(start);

  if (max(1E4, s) / dist < 0.9)
    throw primesieve_error("nth prime < 2 is impossible, negative n is too small");
}

// Prime count approximation
int64_t pix(int64_t n)
{
  double x = static_cast<double>(n);
  double logx = log(max(4.0, x));
  double pix = x / logx;

  return static_cast<int64_t>(pix);
}

bool sieveBackwards(int64_t n, int64_t count, uint64_t stop)
{
  return (count >= n) &&
        !(count == n && stop < 2);
}

uint64_t nthPrimeDist(int64_t n, int64_t count, uint64_t start)
{
  double x = static_cast<double>(n - count);
  double s = static_cast<double>(start);

  x = abs(x);
  x = max(4.0, x);
  s = max(4.0, s);

  double logx = log(x);
  double loglogx = log(logx);
  double pix = x * (logx + loglogx - 1);

  // Correct start if sieving backwards
  if (count >= n)
    s -= pix;

  // Approximate the nth prime using
  // start + n * log(start + pi(n) / loglog(n))
  double startPix = s + pix / loglogx;
  double logStartPix = log(max(4.0, startPix));
  double dist = max(pix, x * logStartPix);
  double maxPrimeGap = logStartPix * logStartPix;
  double sign = (count < n) ? -2 : 2;

  // Make sure start + dist <= nth prime
  dist += sqrt(dist) * log(logStartPix) * sign;
  dist = max(dist, maxPrimeGap);

  if (count >= n)
    checkLowerLimit(start, dist);

  return static_cast<uint64_t>(dist);
}

/// This class is used to generate n primes and
/// then stop by throwing an exception.
///
class NthPrime : public Callback<uint64_t> {
public:
  NthPrime() : n_(0), nthPrime_(0) { }
  void findNthPrime(uint64_t, uint64_t, uint64_t);
  void callback(uint64_t);
  uint64_t getNthPrime() const;
private:
  uint64_t n_;
  uint64_t nthPrime_;
};

void NthPrime::findNthPrime(uint64_t n, uint64_t start, uint64_t stop)
{
  n_ = n;
  PrimeSieve ps;
  try
  {
    ps.callbackPrimes(start, stop, this);
    if (stop < get_max_stop())
      ps.callbackPrimes(stop + 1, get_max_stop(), this);
    throw primesieve_error("nth prime > 2^64");
  }
  catch (cancel_callback&) { }
}

uint64_t NthPrime::getNthPrime() const
{
  return nthPrime_;
}

void NthPrime::callback(uint64_t prime)
{
  if (--n_ == 0)
  {
    nthPrime_ = prime;
    throw cancel_callback();
  }
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
  int64_t tinyN = 10000;
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

  if (n < 0)
    count -= 1;

  checkLimit(start);
  uint64_t overValue = 3;
  dist = nthPrimeDist(n, count, start) * overValue;
  stop = add_overflow_safe(start, dist);
  NthPrime np;
  np.findNthPrime(n - count, start, stop);
  seconds_ = getWallTime() - t1;

  return np.getNthPrime();
}

} // namespace
