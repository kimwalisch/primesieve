///
/// @file   count_primes1.cpp
/// @brief  Count the primes up to 10^9.
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/ParallelSieve.hpp>

#include <stdint.h>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cmath>

using namespace primesieve;

/// Correct pi(x) values to compare with test results
const uint64_t pix[9] =
{
  4,        // pi(10^1)
  25,       // pi(10^2)
  168,      // pi(10^3)
  1229,     // pi(10^4)
  9592,     // pi(10^5)
  78498,    // pi(10^6)
  664579,   // pi(10^7)
  5761455,  // pi(10^8)
  50847534  // pi(10^9)
};

void check(bool OK)
{
  std::cout << "   " << (OK ? "OK" : "ERROR") << "\n";
  if (!OK)
    std::exit(1);
}

int main()
{
  std::cout << std::left;
  ParallelSieve ps;
  ps.setStart(0);
  ps.setStop(0);
  uint64_t count = 0;

  // pi(x) with x = 10^i for i = 1 to 9
  for (int i = 1; i <= 9; i++)
  {
    count += ps.countPrimes(ps.getStop() + 1, (uint64_t) std::pow(10.0, i));
    std::cout << "pi(10^" << i << ") = " << std::setw(12) << count;
    check(count == pix[i - 1]);
  }

  std::cout << std::endl;
  std::cout << "All tests passed successfully!" << std::endl;

  return 0;
}
