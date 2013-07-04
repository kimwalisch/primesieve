///
/// @file  PrimeSieve-nthPrime.cpp
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include "config.h"
#include "PrimeSieve.h"
#include "PrimeSieveCallback.h"
#include "primesieve_error.h"
#include "stop_primesieve.h"
#include "SieveOfEratosthenes.h"

#include <stdint.h>
#include <algorithm>
#include <cmath>

namespace {

using soe::SieveOfEratosthenes;
using std::log;

/// This class is used to generate n primes and
/// then stop by throwing an exception.
///
class NthPrime : public PrimeSieveCallback<uint64_t> {
public:
  NthPrime() : n_(0), nthPrime_(0) { }
  void findNthPrime(uint64_t, uint64_t, uint64_t);
  void callback(uint64_t);
  uint64_t getNthPrime() const;
private:
  uint64_t n_;
  uint64_t nthPrime_;
};

void NthPrime::findNthPrime(uint64_t start, uint64_t stop, uint64_t n)
{
  n_ = n;
  PrimeSieve ps;
  uint64_t maxStop = SieveOfEratosthenes::getMaxStop();
  try {
    ps.generatePrimes(start, stop, this);
    ps.generatePrimes(stop + 1, maxStop, this);
    throw primesieve_error("nth prime is too large > 2^64 - 2^32 * 11");
  }
  catch (stop_primesieve&) { }
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
    throw stop_primesieve();
  }
}

uint64_t pixApproximation(uint64_t n)
{
  if (n < 2) return 0;
  if (n < 3) return 1;

  double x = static_cast<double>(n);
  double logx = log(x);
  return static_cast<uint64_t>(x / (logx - 1));
}

uint64_t nthPrimeDistance(uint64_t start, uint64_t n, double factor = 1.0, double offset = 0.0)
{
  if (n < 1)
    return 0;

  double x = static_cast<double>(n);
  double n2 = static_cast<double>(pixApproximation(start) + n);
  double logx = log(n2);
  double loglogx = log(logx);

  // avoids dist < 0
  if (n2 <= 10)
    return static_cast<uint64_t>(/* p(10) = */ 29 * factor + offset);

  // This formula is more accurate than x * logx
  // http://en.wikipedia.org/wiki/Prime_number_theorem#Approximations_for_the_nth_prime_number
  double dist = x * logx + x * loglogx - x + x * (loglogx - 2) / logx - x * (loglogx * loglogx - 6 * loglogx + 11) / (2 * logx * logx);

  return static_cast<uint64_t>(dist * factor + offset);
}

void checkLimit(uint64_t start, uint64_t dist)
{
  uint64_t maxStop = SieveOfEratosthenes::getMaxStop();
  if (maxStop - start < dist)
    throw primesieve_error("nth prime is too large > 2^64 - 2^32 * 11");
}

} // end namespace

uint64_t PrimeSieve::nthPrime(uint64_t n)
{
  return nthPrime(0, n);
}

uint64_t PrimeSieve::nthPrime(uint64_t start, uint64_t n)
{
  if (n < 1)
    return 0;

  setStart(start);
  double t1 = getWallTime();

  uint64_t stop = 0;
  uint64_t count = 0;
  uint64_t dist = 0;

  // Count the primes up to an approximate nth prime, this step
  // is multi-threaded if ParallelPrimeSieve is used
  while (count < n && (n - count) > 1000000)
  {
    dist = nthPrimeDistance(start, n - count);
    checkLimit(start, dist);
    stop = start + dist;
    count += countPrimes(start, stop);
    start = stop + 1;
  }

  // We have counted more than n primes so rollback
  while (count >= n)
  {
    dist = nthPrimeDistance(start, count - n, 1.2, 10000);
    dist = std::min(dist, stop);
    start = stop - dist;
    count -= countPrimes(start, stop);
    stop = start - 1;
  }

  // Sieve the small remaining distance in arithmetic
  // order using a single thread
  dist = nthPrimeDistance(start, n - count, 2.0, 10000);
  checkLimit(start, dist);
  stop = start + dist;
  NthPrime np;
  np.findNthPrime(start, stop, n - count);
  seconds_ = getWallTime() - t1;

  return np.getNthPrime();
}
