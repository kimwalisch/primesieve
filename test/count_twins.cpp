///
/// @file   count_twins.cpp
/// @brief  Count twin primes inside [10^12, 10^12 + 10^9].
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve.hpp>

#include <cstdint>
#include <iostream>
#include <cstdlib>

void check(bool OK)
{
  std::cout << "   " << (OK ? "OK" : "ERROR") << "\n";
  if (!OK)
    std::exit(1);
}

int main()
{
  uint64_t start = (uint64_t) 1e12;
  uint64_t stop = (uint64_t)(1e12 + 1e9);
  uint64_t count = primesieve::count_twins(start, stop);
  std::cout << "Twin primes inside [10^12, 10^12 + 10^9] = " << count;
  check(count == 1730012);

  std::cout << std::endl;
  std::cout << "Test passed successfully!" << std::endl;

  return 0;
}
