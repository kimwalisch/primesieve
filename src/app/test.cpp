///
/// @file   test.cpp
/// @brief  primesieve self tests (option: --test).
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve.hpp>
#include <ParallelSieve.hpp>
#include <primesieve/Vector.hpp>

#include <stdint.h>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>

using std::size_t;
using namespace primesieve;

namespace {

void check(bool OK)
{
  std::cout << "   " << (OK ? "OK" : "ERROR") << std::endl;

  if (!OK)
  {
    std::cerr << std::endl;
    std::cerr << "Test failed!" << std::endl;
    std::exit(1);
  }
}

void countSmallPrimes()
{
  const Array<uint64_t, 9> primePi =
  {
    4,        // PrimePi(10^1)
    25,       // PrimePi(10^2)
    168,      // PrimePi(10^3)
    1229,     // PrimePi(10^4)
    9592,     // PrimePi(10^5)
    78498,    // PrimePi(10^6)
    664579,   // PrimePi(10^7)
    5761455,  // PrimePi(10^8)
    50847534  // PrimePi(10^9)
  };

  ParallelSieve ps;
  uint64_t count = 0;
  uint64_t stop = 1;

  for (size_t i = 0; i < primePi.size(); i++)
  {
    uint64_t start = stop + 1;
    stop *= 10;
    count += ps.countPrimes(start, stop);
    std::ostringstream oss;
    oss << "PrimePi(10^" << i + 1 << ") = " << count;
    std::cout << std::left << std::setw(24) << oss.str();
    check(count == primePi[i]);
  }
}

void countPrimeKTuplets()
{
  const Array<uint64_t, 5> kTupletCounts =
  {
    17278660, // PrimePi2(10^12, 10^12+10^10)
    2130571,  // PrimePi3(10^13, 10^13+10^10)
    38270,    // PrimePi4(10^14, 10^14+10^10)
    4107,     // PrimePi5(10^15, 10^15+10^10)
    66        // PrimePi6(10^16, 10^16+10^10)
  };

  uint64_t start = (uint64_t) 1e12;
  size_t j = 12;

  for (size_t i = 0; i < kTupletCounts.size(); i++)
  {
    uint64_t stop = start + (uint64_t) 1e10;
    int k = (int) (i + 2);
    int countKTuplet = COUNT_PRIMES << (k - 1);

    ParallelSieve ps;
    ps.addFlags(countKTuplet);
    ps.sieve(start, stop);
    uint64_t count = ps.getCount(k - 1);
    std::ostringstream oss;
    oss << "PrimePi" << k << "(10^" << j << ", 10^" << j << "+10^10) = " << count;
    std::cout << std::left << std::setw(39) << oss.str();
    check(count == kTupletCounts[i]);

    start *= 10;
    j += 1;
  }
}

void countLargePrimes()
{
  const Array<uint64_t, 6> primePi =
  {
    361840208, // PrimePi(10^12, 10^12+10^10)
    334067230, // PrimePi(10^13, 10^13+10^10)
    310208140, // PrimePi(10^14, 10^14+10^10)
    289531946, // PrimePi(10^15, 10^15+10^10)
    271425366, // PrimePi(10^16, 10^16+10^10)
    255481287  // PrimePi(10^17, 10^17+10^10)
  };

  uint64_t start = (uint64_t) 1e12;
  size_t j = 12;

  for (size_t i = 0; i < primePi.size(); i++)
  {
    uint64_t stop = start + (uint64_t) 1e10;
    uint64_t count = count_primes(start, stop);
    std::cout << "PrimePi(10^" << j << ", 10^" << j << "+10^10) = " << count;
    check(count == primePi[i]);

    start *= 10;
    j += 1;
  }
}

void countPrimesRandom()
{
  uint64_t count = 0;
  uint64_t maxDist = (uint64_t) 1e8;
  uint64_t lowerBound = (uint64_t) 1e13;
  uint64_t upperBound = lowerBound + (uint64_t) 1e10;
  uint64_t start = lowerBound - 1;
  uint64_t stop = start;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<uint64_t> dist(0, maxDist);
  int defaultSieveSize = get_sieve_size();

  while (stop < upperBound)
  {
    start = stop + 1;
    stop = std::min(start + dist(gen), upperBound);
    set_sieve_size(1 << (dist(gen) % 14));
    count += count_primes(start, stop);
    std::cout << "\rPrimePi(10^13, 10^13+10^10) = " << count << std::flush;
  }

  check(count == 334067230);
  set_sieve_size(defaultSieveSize);
}

void smallNthPrimes()
{
  const Array<uint64_t, 9> nthPrimes =
  {
    29,         // nthPrime(10^1)
    541,        // nthPrime(10^2)
    7919,       // nthPrime(10^3)
    104729,     // nthPrime(10^4)
    1299709,    // nthPrime(10^5)
    15485863,   // nthPrime(10^6)
    179424673,  // nthPrime(10^7)
    2038074743, // nthPrime(10^8)
    22801763489 // nthPrime(10^9)
  };

  ParallelSieve ps;
  uint64_t n = 1;
  uint64_t nthPrime = 2;

  for (size_t i = 0; i < nthPrimes.size(); i++)
  {
    uint64_t oldN = n;
    uint64_t oldNthPrime = nthPrime;
    n *= 10;
    nthPrime = ps.nthPrime(n - oldN, oldNthPrime);
    std::ostringstream oss;
    oss << "NthPrime(10^" << i + 1 << ") = " << nthPrime;
    std::cout << std::left << std::setw(28) << oss.str();
    check(nthPrime == nthPrimes[i]);
  }
}

} // namespace

void test()
{
  auto t1 = std::chrono::system_clock::now();

  countSmallPrimes();
  std::cout << std::endl;
  countLargePrimes();
  countPrimesRandom();
  std::cout << std::endl;
  countPrimeKTuplets();
  std::cout << std::endl;
  smallNthPrimes();

  auto t2 = std::chrono::system_clock::now();
  std::chrono::duration<double> seconds = t2 - t1;

  std::cout << std::endl;
  std::cout << "All tests passed successfully!" << std::endl;
  std::cout << "Seconds: " << std::fixed << std::setprecision(3) << seconds.count() << std::endl;
}
