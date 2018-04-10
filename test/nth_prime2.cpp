///
/// @file   nth_prime2.cpp
/// @brief  Test nth_prime edge cases
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve.hpp>

#include <stdint.h>
#include <iostream>
#include <exception>
#include <cstdlib>

using namespace std;
using namespace primesieve;

void check(bool OK)
{
  cout << "   " << (OK ? "OK" : "ERROR") << "\n";
  if (!OK)
    exit(1);
}

int main()
{
  uint64_t res;
  uint64_t start;
  int64_t n;

  n = 1;
  start = 1;
  res = nth_prime(n, start);
  cout << "nth_prime(" << n << ", " << start << ") = " << res;
  check(res == 2);

  n = 1;
  start = 2;
  res = nth_prime(n, start);
  cout << "nth_prime(" << n << ", " << start << ") = " << res;
  check(res == 3);

  n = -1;
  start = 102;
  res = nth_prime(n, start);
  cout << "nth_prime(" << n << ", " << start << ") = " << res;
  check(res == 101);

  n = -1;
  start = 101;
  res = nth_prime(n, start);
  cout << "nth_prime(" << n << ", " << start << ") = " << res;
  check(res == 97);

  n = -9592;
  start = 100000;
  res = nth_prime(n, start);
  cout << "nth_prime(" << n << ", " << start << ") = " << res;
  check(res == 2);

  n = -9591;
  start = 100000;
  res = nth_prime(n, start);
  cout << "nth_prime(" << n << ", " << start << ") = " << res;
  check(res == 3);

  try
  {
    n = -1;
    start = 2;
    res = nth_prime(n, start);
    cerr << "ERROR: nth_prime(" << n << ", " << start << ") = " << res;
    return 1;
  }
  catch (primesieve_error& e)
  {
    cout << "OK: " << e.what() << endl;
  }

  try
  {
    n = 1;
    start = 18446744073709551557ull;
    res = nth_prime(n, start);
    cerr << "ERROR: nth_prime(" << n << ", " << start << ") = " << res;
    return 1;
  }
  catch (primesieve_error& e)
  {
    cout << "OK: " << e.what() << endl;
  }

  cout << endl;
  cout << "All tests passed successfully!" << endl;

  return 0;
}
