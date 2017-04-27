///
/// @file   count_primes2.cpp
/// @brief  Count the primes within [10^i, 10^i + 10^8] for i = 12 to 19
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/ParallelPrimeSieve.hpp>

#include <stdint.h>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cmath>

using namespace std;
using namespace primesieve;

const uint64_t pix[8] =
{
  3618282, // pi[10^12, 10^12+10^8]
  3342093, // pi[10^13, 10^13+10^8]
  3102679, // pi[10^14, 10^14+10^8]
  2893937, // pi[10^15, 10^15+10^8]
  2714904, // pi[10^16, 10^16+10^8]
  2555873, // pi[10^17, 10^17+10^8]
  2414886, // pi[10^18, 10^18+10^8]
  2285232  // pi[10^19, 10^19+10^8]
};

void check(bool OK)
{
  cout << "   " << (OK ? "OK" : "ERROR") << "\n";
  if (!OK)
    exit(1);
}

int main()
{
  cout << left;
  ParallelPrimeSieve p;
  p.setFlags(p.COUNT_PRIMES | p.PRINT_STATUS);

  for (int i = 0; i <= 7; i++)
  {
    int j = i + 12;
    cout << "Sieving the primes within [10^" << j << ", 10^" << j << " + 10^8]" << endl;
    p.setStart((uint64_t) pow(10.0, j));
    p.setStop(p.getStart() + (uint64_t) 1e8);
    p.sieve();
    cout << "\rPrime count: " << setw(7) << p.getPrimeCount();
    check(p.getPrimeCount() == pix[i]);
  }

  cout << endl;
  cout << "All tests passed successfully!" << endl;

  return 0;
}
