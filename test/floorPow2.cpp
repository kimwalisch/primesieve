///
/// @file   floorPow2.cpp
/// @brief  Round down to nearest power of 2.
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/pmath.hpp>

#include <stdint.h>
#include <iostream>
#include <cmath>
#include <cstdlib>

void check(bool OK)
{
  std::cout << "   " << (OK ? "OK" : "ERROR") << "\n";
  if (!OK)
    std::exit(1);
}

uint64_t floorPow2_cmath(uint64_t n)
{
  return 1ull << (uint64_t) std::log2(n);
}

int main()
{
  uint64_t n;
  uint64_t res1;
  uint64_t res2;

  for (n = 1; n < 100000; n++)
  {
    res1 = floorPow2(n);
    res2 = floorPow2_cmath(n);
    std::cout << "floorPow2(" << n << ") = " << res1;
    check(res1 == res2);
  }

  n = (1ull << 32) - 1;
  res1 = floorPow2(n);
  res2 = floorPow2_cmath(n);
  std::cout << "floorPow2(" << n << ") = " << res1;
  check(res1 == res2);

  n = 1ull << 32;
  res1 = floorPow2(n);
  res2 = floorPow2_cmath(n);
  std::cout << "floorPow2(" << n << ") = " << res1;
  check(res1 == res2);

  n = (1ull << 63) - 1;
  res1 = floorPow2(n);
  std::cout << "floorPow2(" << n << ") = " << res1;
  check(res1 == (1ull << 62));

  n = 1ull << 63;
  res1 = floorPow2(n);
  std::cout << "floorPow2(" << n << ") = " << res1;
  check(res1 == (1ull << 63));

  n = 18446744073709551615ull;
  res1 = floorPow2(n);
  std::cout << "floorPow2(" << n << ") = " << res1;
  check(res1 == (1ull << 63));

  std::cout << std::endl;
  std::cout << "All tests passed successfully!" << std::endl;

  return 0;
}
