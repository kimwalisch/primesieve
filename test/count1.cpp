///
/// @file   count1.cpp
/// @brief  Count the primes up to 10^10.
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
#include <cstdlib>
#include <cmath>


using namespace std;
using namespace primesieve;

/// Correct pi(x) values to compare with test results
const uint64_t primeCounts[10] =
{
  4,        // pi(10^1)
  25,       // pi(10^2)
  168,      // pi(10^3)
  1229,     // pi(10^4)
  9592,     // pi(10^5)
  78498,    // pi(10^6)
  664579,   // pi(10^7)
  5761455,  // pi(10^8)
  50847534, // pi(10^9)
  455052511 // pi(10^10)
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
    cout << "pi(x) : Prime-counting function test" << endl;
    ParallelPrimeSieve pps;
    pps.setStart(0);
    pps.setStop(0);
    uint64_t primeCount = 0;

    // pi(x) with x = 10^i for i = 1 to 10
    for (int i = 1; i <= 10; i++)
    {
      uint64_t nextStop = (uint64_t) pow(10.0, i);
      primeCount += pps.countPrimes(pps.getStop() + 1, nextStop);
      cout << "pi(10^" << i << (i < 10 ? ")  = " : ") = ") << setw(12) << primeCount;
      check(primeCount == primeCounts[i - 1]);
    }
    cout << endl;
  }
  catch (exception& e)
  {
    cerr << endl << "primesieve error: " << e.what() << endl;
    return 1;
  }
  cout << "All tests passed successfully!" << endl;

  return 0;
}
