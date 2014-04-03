///
/// @file  PrimeSieve-nthPrime.cpp
///
/// Copyright (C) 2014 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/config.hpp>
#include <primesieve/PrimeSieve.hpp>
#include <primesieve/SieveOfEratosthenes.hpp>
#include <primesieve/Callback.hpp>
#include <primesieve/cancel_callback.hpp>
#include <primesieve/primesieve_error.hpp>
#include <primesieve/pmath.hpp>

#include <stdint.h>
#include <algorithm>
#include <cmath>

namespace {

using namespace std;

uint64_t pixGuess(uint64_t stop)
{
  double x = static_cast<double>(stop);
  double logx = log(max(3.0, x));
  double pix = x / logx;

  return static_cast<uint64_t>(pix);
}

uint64_t nthPrimeDistance(uint64_t n, uint64_t start, int direction = 1, double factor = 1.0)
{
  double x = static_cast<double>(n);
  double s = static_cast<double>(start);

  // Avoids log(x) < 1
  x = max(3.0, x);

  // For a given x find dist such that:
  // count_primes(start, start + dist) ~= x
  // This code uses a combination of the two formulas
  // x * log(x) and x * log(start)
  double pix = x * log(x);
  double log1 = log(max(1.0, s / pix));
  double log2 = log(max(1.0, s + x * max(1.0, log1) * direction));
  double dist = x * log2;

  // Make sure start + dist <= nth prime
  dist += sqrt(dist) * (log(max(1.0, log2)) + 3.0) * -direction;
  dist = max(1E4, dist) * factor;

  return static_cast<uint64_t>(dist);
}

} // namespace

namespace primesieve {

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
  uint64_t maxStop = SieveOfEratosthenes::getMaxStop();
  try {
    ps.callbackPrimes(start, stop, this);
    ps.callbackPrimes(stop + 1, maxStop, this);
    throw primesieve_error("nth prime is too large > 2^64 - 2^32 * 11");
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

void checkLimit(uint64_t start, uint64_t dist)
{
  uint64_t maxStop = SieveOfEratosthenes::getMaxStop();
  if (maxStop - start < dist)
    throw primesieve_error("nth prime is too large > 2^64 - 2^32 * 11");
}

uint64_t PrimeSieve::nthPrime(uint64_t n)
{
  return nthPrime(0, n);
}

uint64_t PrimeSieve::nthPrime(uint64_t n, uint64_t start)
{
  if (n < 1)
    return 0;

  setStart(start);
  double t1 = getWallTime();

  uint64_t stop = start + nthPrimeDistance(n, start);
  uint64_t bruteForceMin = 10000;
  uint64_t bruteForceThreshold = std::max(pixGuess(isqrt(stop)), bruteForceMin);
  uint64_t count = 0;
  uint64_t dist = 0;

  // Count the primes up to an approximate nth prime, this step is
  // multi-threaded if ParallelPrimeSieve is used
  while (count < n && (n - count) > bruteForceThreshold)
  {
    dist = nthPrimeDistance(n - count, start);
    checkLimit(start, dist);
    stop = start + dist;
    count += countPrimes(start, stop);
    start = stop + 1;
  }

  // Go back
  while (count >= n)
  {
    dist = nthPrimeDistance(count - n, start, /* backwards */ -1);
    dist = std::min(dist, stop);
    start = stop - dist;
    count -= countPrimes(start, stop);
    stop = start - 1;
  }

  // Sieve the small remaining distance in arithmetic
  // order using a single thread
  dist = nthPrimeDistance(n - count, start, /* forwards */ 1, /* safety factor */ 1.5);
  checkLimit(start, dist);
  stop = start + dist;
  NthPrime np;
  np.findNthPrime(n - count, start, stop);
  seconds_ = getWallTime() - t1;

  return np.getNthPrime();
}

} // namespace primesieve
