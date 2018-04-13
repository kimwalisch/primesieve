///
/// @file   count_primes3.cpp
/// @brief  Count the primes within [10^12, 10^12 + 10^9]
///         using random sized intervals.
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve.hpp>

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
  cout << "Sieving the primes within [10^12, 10^12 + 10^9] randomly" << endl;

  uint64_t count = 0;
  uint64_t maxDist = (uint64_t) 1e7;
  uint64_t lowerBound = (uint64_t) 1e12;
  uint64_t upperBound = lowerBound + (uint64_t) 1e9;
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

    cout << "\rRemaining chunk:             "
         << "\rRemaining chunk: "
         << upperBound - stop << flush;
  }

  cout << endl << "Prime count: " << count;
  check(count == 36190991);

  cout << endl;
  cout << "Test passed successfully!" << endl;

  return 0;
}
