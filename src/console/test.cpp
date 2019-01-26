///
/// @file   test.cpp
/// @brief  primesieve self tests (option: --test).
///
/// Copyright (C) 2019 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve.hpp>
#include <primesieve/ParallelSieve.hpp>

#include <stdint.h>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>

using namespace std;
using namespace primesieve;

namespace {

void check(bool OK)
{
  cout << "   " << (OK ? "OK" : "ERROR") << endl;

  if (!OK)
  {
    cerr << endl;
    cerr << "Test failed!" << endl;
    exit(1);
  }
}

void countSmallPrimes()
{
  const array<uint64_t, 9> primePi =
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
  ps.setStart(0);
  ps.setStop(0);
  uint64_t count = 0;

  for (size_t i = 0; i < primePi.size(); i++)
  {
    uint64_t start = ps.getStop() + 1;
    uint64_t stop = (uint64_t) pow(10.0, i + 1);
    count += ps.countPrimes(start, stop);
    ostringstream oss;
    oss << "PrimePi(10^" << i + 1 << ") = " << count;
    cout << left << setw(24) << oss.str();
    check(count == primePi[i]);
  }
}

void countPrimeKTuplets()
{
  const array<uint64_t, 5> kTupletCounts =
  {
    17278660, // PrimePi2(10^12, 10^12+10^10)
    2130571,  // PrimePi3(10^13, 10^13+10^10)
    38270,    // PrimePi4(10^14, 10^14+10^10)
    4107,     // PrimePi5(10^15, 10^15+10^10)
    66        // PrimePi6(10^16, 10^16+10^10)
  };

  for (size_t i = 0; i < kTupletCounts.size(); i++)
  {
    size_t j = i + 12;
    uint64_t start = (uint64_t) pow(10.0, j);
    uint64_t stop = start + (uint64_t) 1e10;
    int k = (int) (i + 2);
    int countKTuplet = COUNT_PRIMES << (k - 1);

    ParallelSieve ps;
    ps.addFlags(countKTuplet);
    ps.sieve(start, stop);
    uint64_t count = ps.getCount(k - 1);
    ostringstream oss;
    oss << "PrimePi" << k << "(10^" << j << ", 10^" << j << "+10^10) = " << count;
    cout << left << setw(39) << oss.str();
    check(count == kTupletCounts[i]);
  }
}

void countLargePrimes()
{
  const array<uint64_t, 6> primePi =
  {
    361840208, // PrimePi(10^12, 10^12+10^10)
    334067230, // PrimePi(10^13, 10^13+10^10)
    310208140, // PrimePi(10^14, 10^14+10^10)
    289531946, // PrimePi(10^15, 10^15+10^10)
    271425366, // PrimePi(10^16, 10^16+10^10)
    255481287  // PrimePi(10^17, 10^17+10^10)
  };

  for (size_t i = 0; i < primePi.size(); i++)
  {
    size_t j = i + 12;
    uint64_t start = (uint64_t) pow(10.0, j);
    uint64_t stop = start + (uint64_t) 1e10;
    uint64_t count = count_primes(start, stop);
    cout << "PrimePi(10^" << j << ", 10^" << j << "+10^10) = " << count;
    check(count == primePi[i]);
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

  random_device rd;
  mt19937 gen(rd());
  uniform_int_distribution<uint64_t> dist(0, maxDist);

  while (stop < upperBound)
  {
    start = stop + 1;
    stop = min(start + dist(gen), upperBound);
    set_sieve_size(1 << (dist(gen) % 13));
    count += count_primes(start, stop);
    cout << "\rPrimePi(10^13, 10^13+10^10) = " << count << flush;
  }

  check(count == 334067230);
}

} // namespace

void test()
{
  auto t1 = chrono::system_clock::now();

  countSmallPrimes();
  cout << endl;
  countPrimeKTuplets();
  cout << endl;
  countLargePrimes();
  countPrimesRandom();

  auto t2 = chrono::system_clock::now();
  chrono::duration<double> seconds = t2 - t1;

  cout << endl;
  cout << "All tests passed successfully!" << endl;
  cout << "Seconds: " << fixed << setprecision(3) << seconds.count() << endl;

  exit(0);
}
