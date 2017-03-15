///
/// @file   nth_prime3.cpp
/// @brief  Long distance nth prime testing.
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve.hpp>

#include <stdint.h>
#include <iostream>
#include <cstdlib>

using namespace std;
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
      cerr << endl;
      cerr << "nth_prime(" << -n << ", " << i_start << ") = " << prime << "   ERROR" << endl;
      exit(1);
    }
  }
}

int main()
{
  for (int i = 3; i <= 7; i++)
  {
    for (int j = 10; j <= 13; j++)
    {
      int64_t n = (int64_t) pow(10.0, i);
      int64_t start = (int64_t) pow(10.0, j);
      int64_t iters = 5;
      
      cout << "nth_prime_test(" << n << ", " << start << ", " << iters << ")";
      nth_prime_test(n, start, iters);
      cout << " = OK" << endl;
    }
  }

  cout << endl;
  cout << "All tests passed successfully!" << endl;

  return 0;
}
