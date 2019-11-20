///
/// @file   isqrt.cpp
/// @brief  Test integer square root function.
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/pmath.hpp>

#include <stdint.h>
#include <iostream>
#include <cmath>
#include <cstdlib>

using namespace std;

void check(bool OK)
{
  cout << "   " << (OK ? "OK" : "ERROR") << "\n";
  if (!OK)
    exit(1);
}

int main()
{
  uint64_t n;
  uint64_t res1;
  double res2;

  for (n = 0; n < 100000; n++)
  {
    res1 = isqrt(n);
    res2 = sqrt((double) n);
    cout << "isqrt(" << n << ") = " << res1;
    check(res1 == (uint64_t) res2);
  }

  n = (1ull << 32) - 1;
  res1 = isqrt(n);
  res2 = sqrt((double) n);
  cout << "isqrt(" << n << ") = " << res1;
  check(res1 == (uint64_t) res2);

  n = 1ull << 32;
  res1 = isqrt(n);
  res2 = sqrt((double) n);
  cout << "isqrt(" << n << ") = " << res1;
  check(res1 == (uint64_t) res2);

  n = 1000000000000000000ull - 1;
  res1 = isqrt(n);
  cout << "isqrt(" << n << ") = " << res1;
  check(res1 == 999999999);

  n = 1000000000000000000ull;
  res1 = isqrt(n);
  cout << "isqrt(" << n << ") = " << res1;
  check(res1 == 1000000000);

  n = 18446744073709551615ull;
  res1 = isqrt(n);
  cout << "isqrt(" << n << ") = " << res1;
  check(res1 == 4294967295ull);

  cout << endl;
  cout << "All tests passed successfully!" << endl;

  return 0;
}
