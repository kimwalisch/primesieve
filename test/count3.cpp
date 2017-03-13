///
/// @file   count3.cpp
/// @brief  Count the primes within [10^13, 10^13 + 5*10^9] randomly
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve.hpp>

#include <stdint.h>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <algorithm>
#include <random>

using namespace std;
using namespace primesieve;

void check(bool isCorrect)
{
  cout << "   " << (isCorrect ? "OK" : "ERROR") << endl;
  if (!isCorrect)
    throw runtime_error("test failed!");
}

int main()
{
  try
  {
    cout << "Sieving the primes within [10^13, 10^13 + 5*10^9] randomly" << endl;
    uint64_t maxDist = (uint64_t) 2e7;
    uint64_t lowerBound = (uint64_t) 1e13;
    uint64_t upperBound = lowerBound + (uint64_t) 5e9;
    uint64_t primeCount = 0;
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
      p.setSieveSize(1 << (dist(gen) % 12));
      p.sieve();
      primeCount += p.getPrimeCount();
      cout << "\rRemaining chunk:             "
           << "\rRemaining chunk: "
           << upperBound - p.getStop() << flush;
    }
    cout << endl << "Prime count: " << primeCount;
    check(primeCount == 167038410);
    cout << endl;
    cout << "Test passed successfully!" << endl;
  }
  catch (exception& e)
  {
    cerr << endl << "primesieve error: " << e.what() << endl;
    return 1;
  }

  return 0;
}
