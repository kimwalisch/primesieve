///
/// @file   nth_prime3.cpp
/// @brief  Long distance nth prime testing.
///
/// Copyright (C) 2024 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve.hpp>

#include <stdint.h>
#include <iostream>
#include <cstdlib>

using namespace primesieve;

void nth_prime_test(int64_t n, int64_t start, int64_t iters)
{
  for (int64_t i = 1; i <= iters; i++)
  {
    int64_t i_start = (i * start) + 1;
    int64_t prime = nth_prime(-n, i_start);
    int64_t smaller = nth_prime(n, prime - 1);
    int64_t larger = nth_prime(n, prime);

    if (i_start <= smaller || i_start > larger)
    {
      std::cerr << std::endl;
      std::cerr << "nth_prime(" << -n << ", " << i_start << ") = " << prime << "   ERROR" << std::endl;
      std::exit(1);
    }
  }
}

int main()
{
  // Set a small sieve size in order to
  // ensure many segments are sieved
  set_sieve_size(16);
  int64_t n = 100;

  for (int i = 3; i <= 6; i++)
  {
    n *= 10;
    int64_t start = (int64_t) 1e7;
    int64_t iters = 5;

    for (int j = 8; j <= 10; j++)
    {
      start *= 10;
      std::cout << "nth_prime_test(" << n << ", " << start << ", " << iters << ")";
      nth_prime_test(n, start, iters);
      std::cout << " = OK" << std::endl;
    }
  }

  std::cout << std::endl;
  std::cout << "All tests passed successfully!" << std::endl;

  return 0;
}
