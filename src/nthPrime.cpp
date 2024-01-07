///
/// @file  nthPrime.cpp
///
/// Copyright (C) 2024 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/iterator.hpp>
#include <primesieve/forward.hpp>
#include <primesieve/PrimeSieve.hpp>
#include <primesieve/macros.hpp>
#include <primesieve/pmath.hpp>
#include <primesieve/primesieve_error.hpp>
#include <primesieve/nthPrimeApprox.hpp>

#include <stdint.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <string>

namespace {

/// PrimePi(2^64)
const uint64_t max_n = 425656284035217743ull;

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
  int64_t countApprox = 0;
  uint64_t primeApprox = 0;
  uint64_t avgPrimeGap = 0;
  uint64_t prime = 0;

  if (start == 0)
    primeApprox = nthPrimeApprox(n);
  else
  {
    ASSERT(start > 0);
    uint64_t nApprox = checkedAdd(primesApprox(start), n);
    nApprox = std::min(nApprox, max_n);
    primeApprox = nthPrimeApprox(nApprox);
  }

  primeApprox = std::max(primeApprox, start);
  if (primeApprox > 0)
    avgPrimeGap = ilog(primeApprox) + 2;

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
  }

  // Here we are very close to the nth prime < sqrt(nth_prime),
  // we simply iterate over the primes until we find it.
  if (countApprox < n)
  {
    uint64_t start = primeApprox + 1;
    uint64_t stop = checkedAdd(start, (n - countApprox) * avgPrimeGap);
    primesieve::iterator iter(start, stop);
    int64_t i = countApprox;
    while (i < n)
    {
      prime = iter.next_prime();
      i += 1;
      if_unlikely(i < n && prime == 18446744073709551557ull)
        throw primesieve_error("nth_prime(n) > 2^64 is not supported!");
    }
  }
  else // if (countApprox >= n)
  {
    uint64_t start = primeApprox;
    uint64_t stop = checkedSub(start, (countApprox - n) * avgPrimeGap);
    primesieve::iterator iter(start, stop);
    for (int64_t i = countApprox; i >= n; i--)
      prime = iter.prev_prime();
  }

  auto t2 = std::chrono::system_clock::now();
  std::chrono::duration<double> seconds = t2 - t1;
  seconds_ = seconds.count();

  return prime;
}

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
  uint64_t nApprox = checkedSub(primesApprox(start), n);
  uint64_t primeApprox = nthPrimeApprox(nApprox);
  uint64_t avgPrimeGap = 0;
  uint64_t prime = 0;

  // Count primes < start
  start = checkedSub(start, 1);
  primeApprox = std::min(primeApprox, start);
  int64_t countApprox = countPrimes(primeApprox, start);
  if (primeApprox > 0)
    avgPrimeGap = ilog(primeApprox) + 2;

  // Here we are very close to the nth prime < sqrt(nth_prime),
  // we simply iterate over the primes until we find it.
  if (countApprox >= n)
  {
    uint64_t start = primeApprox;
    uint64_t stop = checkedAdd(start, (n - countApprox) * avgPrimeGap);
    primesieve::iterator iter(start, stop);
    for (int64_t i = countApprox; i >= n; i--)
      prime = iter.next_prime();
  }
  else // if (countApprox < n)
  {
    uint64_t start = checkedSub(primeApprox, 1);
    uint64_t stop = checkedSub(start, (countApprox - n) * avgPrimeGap);
    primesieve::iterator iter(start, stop);
    for (int64_t i = countApprox; i < n; i += 1)
    {
      prime = iter.prev_prime();
      if_unlikely(prime == 0)
        throw primesieve_error("nth_prime(n): n is too small, nth_prime(n) < 2!");
    }
  }

  auto t2 = std::chrono::system_clock::now();
  std::chrono::duration<double> seconds = t2 - t1;
  seconds_ = seconds.count();

  return prime;
}

} // namespace
