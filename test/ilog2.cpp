///
/// @file   ilog2.cpp
/// @brief  Test integer log2 function.
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

uint64_t log2_cmath(uint64_t n)
{
  return log((double) n) / log(2.0);
}

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
  uint64_t res2;

  for (n = 1; n < 100000; n++)
  {
    res1 = ilog2(n);
    res2 = log2_cmath(n);
    cout << "ilog2(" << n << ") = " << res1;
    check(res1 == res2);
  }

  n = (1ull << 32) - 1;
  res1 = ilog2(n);
  res2 = log2_cmath(n);
  cout << "ilog2(" << n << ") = " << res1;
  check(res1 == (uint64_t) res2);

  n = 1ull << 32;
  res1 = ilog2(n);
  res2 = log2_cmath(n);
  cout << "ilog2(" << n << ") = " << res1;
  check(res1 == (uint64_t) res2);

  n = (1ull << 63) - 1;
  res1 = ilog2(n);
  cout << "ilog2(" << n << ") = " << res1;
  check(res1 == 62);

  n = 1ull << 63;
  res1 = ilog2(n);
  cout << "ilog2(" << n << ") = " << res1;
  check(res1 == 63);

  n = 18446744073709551615ull;
  res1 = ilog2(n);
  cout << "ilog2(" << n << ") = " << res1;
  check(res1 == 63);

  cout << endl;
  cout << "All tests passed successfully!" << endl;

  return 0;
}
