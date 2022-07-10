///
/// @file   clear_primesieve_iterator1.cpp
/// @brief  Test next_prime() of primesieve::iterator.
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve.hpp>

#include <stdint.h>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <vector>

void check(bool OK)
{
  std::cout << "   " << (OK ? "OK" : "ERROR") << "\n";
  if (!OK)
    std::exit(1);
}

int main()
{
  primesieve::iterator it;
  uint64_t primes = 0;

  for (int i = 0; i < 10; i++)
  {
    it.clear();
    uint64_t prime = it.next_prime();
    for (; prime < 100000; prime = it.next_prime())
      primes++;
  }

  std::cout << "Count of the primes = " << primes;
  check(primes == 9592 * 10);

  std::cout << std::endl;
  std::cout << "All tests passed successfully!" << std::endl;

  return 0;
}
