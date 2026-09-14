///
/// @file   generate_primes1.cpp
/// @brief  Test prime number generation.
///
/// Copyright (C) 2026 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve.hpp>

#include <stdint.h>
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>

using namespace primesieve;

// primes inside [0, 100]
const uint64_t small_primes[25] =
{
   2,  3,  5,  7, 11,
  13, 17, 19, 23, 29,
  31, 37, 41, 43, 47,
  53, 59, 61, 67, 71,
  73, 79, 83, 89, 97
};

// primes inside [10^15, 10^15 + 741]
const uint64_t large_primes[19] =
{
  1000000000000037,
  1000000000000091,
  1000000000000159,
  1000000000000187,
  1000000000000223,
  1000000000000241,
  1000000000000249,
  1000000000000259,
  1000000000000273,
  1000000000000279,
  1000000000000297,
  1000000000000357,
  1000000000000399,
  1000000000000403,
  1000000000000487,
  1000000000000513,
  1000000000000613,
  1000000000000711,
  1000000000000741
};

void check(bool OK)
{
  std::cout << "   " << (OK ? "OK" : "ERROR") << "\n";
  if (!OK)
    std::exit(1);
}

int main()
{
  std::vector<uint64_t> primes;
  generate_primes(100, &primes);
  std::cout << "primes.size() = " << primes.size();
  check(primes.size() == 25);

  for (uint64_t i = 0; i < primes.size(); i++)
  {
    std::cout << "primes[" << i << "] = " << primes[i];
    check(primes[i] == small_primes[i]);
  }

  primes.clear();

  generate_primes(1000000000000000, 1000000000000741, &primes);
  std::cout << "primes.size() = " << primes.size();
  check(primes.size() == 19);

  for (uint64_t i = 0; i < primes.size(); i++)
  {
    std::cout << "primes[" << i << "] = " << primes[i];
    check(primes[i] == large_primes[i]);
  }

  std::string errorMsg;

  try
  {
    std::vector<uint16_t> primes16;
    generate_primes((1 << 16) + 10, &primes16);
  }
  catch (const primesieve_error& e)
  {
    errorMsg = e.what();
  }

  std::cout << "Detect 16-bit overflow: " << errorMsg;
  check(!errorMsg.empty());

  std::cout << std::endl;
  std::cout << "All tests passed successfully!" << std::endl;

  return 0;
}
