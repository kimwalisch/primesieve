///
/// @file  nthPrime.cpp
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include "PrimeSieveClass.hpp"
#include "RiemannR.hpp"

#include <primesieve/iterator.hpp>
#include <primesieve/macros.hpp>
#include <primesieve/pmath.hpp>
#include <primesieve/primesieve_error.hpp>

#include <stdint.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <string>

namespace {

/// PrimePi(2^64)
const uint64_t max_n = 425656284035217743ull;

/// Average prime gap near n
uint64_t avgPrimeGap(uint64_t n)
{
  double x = (double) n;
  x = std::max(8.0, x);
  double logx = std::log(x);
  // When we buffer primes using primesieve::iterator, we
  // want to make sure we buffer primes up to the nth
  // prime. Therefore we use +2 here, better to buffer
  // slightly too many primes than not enough primes.
  double primeGap = logx + 2;
  return (uint64_t) primeGap;
}

} // namespace

namespace primesieve {

uint64_t PrimeSieve::nthPrime(uint64_t n)
{
  return nthPrime(0, n);
}

uint64_t PrimeSieve::nthPrime(int64_t n, uint64_t start)
{
  if (n < 0)
    return negativeNthPrime(n, start);
  else if (n == 0)
    n = 1; // like Mathematica
  else if ((uint64_t) n > max_n)
    throw primesieve_error("nth_prime(n): n must be <= " + std::to_string(max_n));

  setStart(start);
  auto t1 = std::chrono::system_clock::now();
  uint64_t nApprox = checkedAdd(primePiApprox(start), n);
  nApprox = std::min(nApprox, max_n);
  uint64_t primeApprox = nthPrimeApprox(nApprox);
  primeApprox = std::max(primeApprox, start);
  int64_t countApprox = 0;
  uint64_t prime = 0;

  // Only use multi-threading if the sieving distance is sufficiently
  // large. For small n this if statement also avoids calling
  // countPrimes() and hence the initialization overhead of
  // O(x^0.5 log log x^0.5) occurs only once (instead of twice) when
  // using primesieve::iterator further down.
  if (primeApprox - start > isqrt(primeApprox) / 10)
  {
    // Count primes > start
    start = checkedAdd(start, 1);
    primeApprox = std::max(start, primeApprox);
    countApprox = countPrimes(start, primeApprox);
    start = primeApprox;
  }

  // Here we are very close to the nth prime < sqrt(nth_prime),
  // we simply iterate over the primes until we find it.
  if (countApprox < n)
  {
    start = checkedAdd(start, 1);
    uint64_t dist = (n - countApprox) * avgPrimeGap(primeApprox);
    uint64_t stop = checkedAdd(start, dist);
    primesieve::iterator iter(start, stop);
    for (int64_t i = countApprox; i < n; i++)
      prime = iter.next_prime();
  }
  else // if (countApprox >= n)
  {
    uint64_t dist = (countApprox - n) * avgPrimeGap(primeApprox);
    uint64_t stop = checkedSub(start, dist);
    primesieve::iterator iter(start, stop);
    for (int64_t i = countApprox; i >= n; i--)
    {
      prime = iter.prev_prime();
      if_unlikely(prime == 0)
        throw primesieve_error("nth_prime(n): invalid n, nth prime < 2 is impossible!");
    }
  }

  auto t2 = std::chrono::system_clock::now();
  std::chrono::duration<double> seconds = t2 - t1;
  seconds_ = seconds.count();

  return prime;
}

/// Used for n < 0
uint64_t PrimeSieve::negativeNthPrime(int64_t n, uint64_t start)
{
  ASSERT(n < 0);
  n = -n;

  if ((uint64_t) n >= start)
    throw primesieve_error("nth_prime(n): abs(n) must be < start");
  else if ((uint64_t) n > max_n)
    throw primesieve_error("nth_prime(n): abs(n) must be <= " + std::to_string(max_n));

  setStart(start);
  auto t1 = std::chrono::system_clock::now();
  uint64_t nApprox = checkedSub(primePiApprox(start), n);
  nApprox = std::min(nApprox, max_n);
  uint64_t primeApprox = nthPrimeApprox(nApprox);
  primeApprox = std::min(primeApprox, start);
  uint64_t prime = 0;
  int64_t countApprox = 0;

  // Only use multi-threading if the sieving distance is sufficiently
  // large. For small n this if statement also avoids calling
  // countPrimes() and hence the initialization overhead of
  // O(x^0.5 log log x^0.5) occurs only once (instead of twice) when
  // using primesieve::iterator further down.
  if (start - primeApprox > isqrt(start) / 10)
  {
    // Count primes < start
    start = checkedSub(start, 1);
    primeApprox = std::min(primeApprox, start);
    countApprox = countPrimes(primeApprox, start);
    start = primeApprox;
  }

  // Here we are very close to the nth prime < sqrt(nth_prime),
  // we simply iterate over the primes until we find it.
  if (countApprox >= n)
  {
    uint64_t dist = (countApprox - n) * avgPrimeGap(start);
    uint64_t stop = checkedAdd(start, dist);
    primesieve::iterator iter(start, stop);
    for (int64_t i = countApprox; i >= n; i--)
      prime = iter.next_prime();
  }
  else // if (countApprox < n)
  {
    start = checkedSub(start, 1);
    uint64_t dist = (n - countApprox) * avgPrimeGap(start);
    uint64_t stop = checkedSub(start, dist);
    primesieve::iterator iter(start, stop);
    for (int64_t i = countApprox; i < n; i++)
    {
      prime = iter.prev_prime();
      if_unlikely(prime == 0)
        throw primesieve_error("nth_prime(n): invalid n, nth prime < 2 is impossible!");
    }
  }

  auto t2 = std::chrono::system_clock::now();
  std::chrono::duration<double> seconds = t2 - t1;
  seconds_ = seconds.count();

  return prime;
}

} // namespace
