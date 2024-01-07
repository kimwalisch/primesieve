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
const int64_t max_n = 425656284035217743ull;

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
  else if (n > max_n)
    throw primesieve_error("nth_prime(n): n must be <= " + std::to_string(max_n));

  setStart(start);
  auto t1 = std::chrono::system_clock::now();
  uint64_t prime_approx;

  if (start == 0) 
    prime_approx = nthPrimeApprox(n);
  else
  {
    ASSERT(start > 0);
    uint64_t new_n = checkedAdd(primesApprox(start), n);
    new_n = std::min(new_n, max_n);
    prime_approx = nthPrimeApprox(new_n);
  }

  start = checkedAdd(start, 1);
  int64_t count_approx = countPrimes(start, prime_approx);
  uint64_t avg_prime_gap = ilog(prime_approx) + 2;
  uint64_t prime = -1;

  // Here we are very close to the nth prime < sqrt(nth_prime),
  // we simply iterate over the primes until we find it.
  if (count_approx < n)
  {
    uint64_t start = prime_approx + 1;
    uint64_t stop = checkedAdd(start, (n - count_approx) * avg_prime_gap);
    primesieve::iterator iter(start, stop);
    int64_t i = count_approx;
    while (i < n)
    {
      prime = iter.next_prime();
      i += 1;
      if_unlikely(i < n && prime == 18446744073709551557ull)
        throw primesieve_error("nth_prime(n) > 2^64 is not supported!");
    }
  }
  else // if (count_approx >= n)
  {
    uint64_t start = prime_approx;
    uint64_t stop = checkedSub(start, (count_approx - n) * avg_prime_gap);
    primesieve::iterator iter(start, stop);
    for (int64_t i = count_approx; i + 1 > n; i--)
      prime = iter.prev_prime();
  }

  auto t2 = std::chrono::system_clock::now();
  std::chrono::duration<double> seconds = t2 - t1;
  seconds_ = seconds.count();

  return prime;
}

} // namespace
