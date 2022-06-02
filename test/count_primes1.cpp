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
#include <primesieve/pod_vector.hpp>

#include <stdint.h>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <iomanip>

using namespace primesieve;

/// Correct pi(x) values to compare with test results
const pod_array<uint64_t, 9> pix =
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

  // pi(x) with x = 10^(i+1)
  for (size_t i = 0; i < pix.size(); i++)
  {
    count += ps.countPrimes(ps.getStop() + 1, (uint64_t) std::pow(10.0, i + 1));
    std::cout << "pi(10^" << i + 1 << ") = " << std::setw(12) << count;
    check(count == pix[i]);
  }

  // Test PreSieve with bufferPrimes < 100.
  // The thread interval must be sufficiently large
  // otherwise minimal pre-sieving is used.
  // Using a single thread increases thread interval.
  ps.setNumThreads(1);
  count = ps.countPrimes(0, (uint64_t) std::pow(10.0, 9));
  std::cout << "pi(10^9) = " << std::setw(12) << count;
  check(count == 50847534);

  std::cout << std::endl;
  std::cout << "All tests passed successfully!" << std::endl;

  return 0;
}
