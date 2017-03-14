///
/// @file   count2.cpp
/// @brief  Count the primes within [10^i, 10^i + 10^9] for i = 12 to 19
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve.hpp>

#include <stdint.h>
#include <iostream>
#include <iomanip>
#include <exception>
#include <stdexcept>
#include <cmath>

using namespace std;
using namespace primesieve;

const uint64_t primeCounts[19] =
{
  36190991, // pi[10^12, 10^12+10^9]
  33405006, // pi[10^13, 10^13+10^9]
  31019409, // pi[10^14, 10^14+10^9]
  28946421, // pi[10^15, 10^15+10^9]
  27153205, // pi[10^16, 10^16+10^9]
  25549226, // pi[10^17, 10^17+10^9]
  24127085, // pi[10^18, 10^18+10^9]
  22854258  // pi[10^19, 10^19+10^9]
};

void check(bool isCorrect)
{
  cout << (isCorrect ? "OK" : "ERROR") << endl;
  if (!isCorrect)
    throw runtime_error("test failed!");
}

int main()
{
  try
  {
    cout << left;
    ParallelPrimeSieve p;
    p.setFlags(p.COUNT_PRIMES | p.PRINT_STATUS);

    for (int i = 0; i <= 7; i++)
    {
      int j = i + 12;
      cout << "Sieving the primes within [10^" << j << ", 10^" << j << " + 10^9]" << endl;
      p.setStart((uint64_t) pow(10.0, j));
      p.setStop(p.getStart() + (uint64_t) 1e9);
      p.sieve();
      cout << "\rPrime count: " << setw(11) << p.getPrimeCount();
      check(p.getPrimeCount() == primeCounts[i]);
    }
    cout << endl;
    cout << "All tests passed successfully!" << endl;
  }
  catch (exception& e)
  {
    cerr << endl << "primesieve error: " << e.what() << endl;
    return 1;
  }

  return 0;
}
