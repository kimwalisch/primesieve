///
/// @file   count_primes3.cpp
/// @brief  Count the primes within [10^13, 10^13 + 5*10^9] randomly
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/ParallelPrimeSieve.hpp>

#include <stdint.h>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <random>

using namespace std;
using namespace primesieve;

void check(bool OK)
{
  cout << "   " << (OK ? "OK" : "ERROR") << "\n";
  if (!OK)
    exit(1);
}

int main()
{
  cout << "Sieving the primes within [10^13, 10^13 + 5*10^9] randomly" << endl;

  uint64_t maxDist = (uint64_t) 2e7;
  uint64_t lowerBound = (uint64_t) 1e13;
  uint64_t upperBound = lowerBound + (uint64_t) 5e9;
  uint64_t count = 0;

  ParallelPrimeSieve p;
  p.setStart(lowerBound - 1);
  p.setStop(lowerBound - 1);

  random_device rd;
  mt19937 gen(rd());
  uniform_int_distribution<uint64_t> dist(0, maxDist);

  while (p.getStop() < upperBound)
  {
    p.setStart(p.getStop() + 1);
    p.setStop(min(p.getStart() + dist(gen), upperBound));
    p.setSieveSize(1 << (dist(gen) % 13));
    p.sieve();
    count += p.getPrimeCount();

    cout << "\rRemaining chunk:             "
         << "\rRemaining chunk: "
         << upperBound - p.getStop() << flush;
  }

  cout << endl << "Prime count: " << count;
  check(count == 167038410);

  cout << endl;
  cout << "Test passed successfully!" << endl;

  return 0;
}
