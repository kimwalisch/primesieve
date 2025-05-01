///
/// @file   Riemann_R.cpp
/// @brief  Test the Riemann R function.
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <RiemannR.hpp>
#include <primesieve/Vector.hpp>

#include <stdint.h>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <limits>

using std::size_t;
using namespace primesieve;

/// Generated using Mathematica:
/// Table[IntegerPart[RiemannR[k]], {k, 0, 99}]
Array<uint64_t, 100> RiemannR_tiny =
{
  0, 1, 1, 2, 2, 2, 3, 3, 3, 4,
  4, 4, 5, 5, 5, 6, 6, 6, 6, 7,
  7, 7, 8, 8, 8, 8, 9, 9, 9, 9,
  10, 10, 10, 10, 11, 11, 11, 11, 12, 12,
  12, 12, 13, 13, 13, 13, 14, 14, 14, 14,
  14, 15, 15, 15, 15, 16, 16, 16, 16, 17,
  17, 17, 17, 17, 18, 18, 18, 18, 18, 19,
  19, 19, 19, 20, 20, 20, 20, 20, 21, 21,
  21, 21, 21, 22, 22, 22, 22, 23, 23, 23,
  23, 23, 24, 24, 24, 24, 24, 25, 25, 25
};

Array<uint64_t, 14> RiemannR_table =
{
                     4, // RiemannR(10^1)
                    25, // RiemannR(10^2)
                   168, // RiemannR(10^3)
                  1226, // RiemannR(10^4)
                  9587, // RiemannR(10^5)
                 78527, // RiemannR(10^6)
                664667, // RiemannR(10^7)
               5761551, // RiemannR(10^8)
              50847455, // RiemannR(10^9)
             455050683, // RiemannR(10^10)
          4118052494ll, // RiemannR(10^11)
         37607910542ll, // RiemannR(10^12)
        346065531065ll, // RiemannR(10^13)
       3204941731601ll  // RiemannR(10^14)
};

void check(bool OK)
{
  std::cout << "   " << (OK ? "OK" : "ERROR") << "\n";
  if (!OK)
    std::exit(1);
}

int main()
{
  for (size_t x = 0; x < RiemannR_tiny.size(); x++)
  {
    std::cout << "RiemannR(" << x << ") = " << (uint64_t) RiemannR((long double) x);
    check((uint64_t) RiemannR((long double) x) == RiemannR_tiny[x]);
  }

  uint64_t x = 1;
  for (size_t i = 0; i < RiemannR_table.size(); i++)
  {
    x *= 10;
    std::cout << "RiemannR(" << x << ") = " << (uint64_t) RiemannR((long double) x);
    check((uint64_t) RiemannR((long double) x) == RiemannR_table[i]);
  }

  std::cout << "RiemannR_inverse(1) = " << RiemannR_inverse((long double) 1);
  check((uint64_t) RiemannR_inverse((long double) 1) == 1);

  for (x = 2; x < RiemannR_tiny.size(); x++)
  {
    uint64_t y = RiemannR_tiny[x];
    std::cout << "RiemannR_inverse(" << y << ") = " << (uint64_t) RiemannR_inverse((long double) y);
    check((uint64_t) RiemannR_inverse((long double) y) < x &&
          (uint64_t) RiemannR_inverse((long double) y + 1) >= x);
  }

  x = 1;
  for (size_t i = 0; i < RiemannR_table.size(); i++)
  {
    x *= 10;
    uint64_t y = RiemannR_table[i];
    std::cout << "RiemannR_inverse(" << y << ") = " << (uint64_t) RiemannR_inverse((long double) y);
    check((uint64_t) RiemannR_inverse((long double) y) < x &&
          (uint64_t) RiemannR_inverse((long double) y + 1) >= x);
  }

  // Sanity checks for tiny values of RiemannR(x)
  for (x = 0; x < 10000; x++)
  {
    uint64_t rix = (uint64_t) RiemannR((long double) x);
    double logx = std::log(std::max((double) x, 2.0));

    if ((x >= 20 && rix < x / logx) ||
        (x >= 2  && rix > x * logx))
    {
      std::cout << "RiemannR(" << x << ") = " << rix << "   ERROR" << std::endl;
      std::exit(1);
    }
  }

  // Sanity checks for small values of RiemannR(x)
  for (; x < 100000; x += 101)
  {
    uint64_t rix = (uint64_t) RiemannR((long double) x);
    double logx = std::log(std::max((double) x, 2.0));

    if ((x >= 20 && rix < x / logx) ||
        (x >= 2  && rix > x * logx))
    {
      std::cout << "RiemannR(" << x << ") = " << rix << "   ERROR" << std::endl;
      std::exit(1);
    }
  }

  // Sanity checks for tiny values of RiemannR_inverse(x)
  for (x = 2; x < 1000; x++)
  {
    uint64_t res = (uint64_t) RiemannR_inverse((long double) x);
    double logx = std::log((double) x);

    if (res < x ||
        (x >= 5 && res > x * logx * logx))
    {
      std::cout << "RiemannR_inverse(" << x << ") = " << res << "   ERROR" << std::endl;
      std::exit(1);
    }
  }

  // Sanity checks for small values of RiemannR_inverse(x)
  for (; x < 100000; x += 101)
  {
    uint64_t res = (uint64_t) RiemannR_inverse((long double) x);
    double logx = std::log((double) x);

    if (res < x ||
        (x >= 5 && res > x * logx * logx))
    {
      std::cout << "RiemannR_inverse(" << x << ") = " << res << "   ERROR" << std::endl;
      std::exit(1);
    }
  }

  {
    uint64_t x = std::numeric_limits<uint64_t>::max() / 10;
    uint64_t res = nthPrimeApprox(x);
    if (res != std::numeric_limits<uint64_t>::max())
    {
      std::cout << "nthPrimeApprox(" << x << ") != UINT64_MAX, failed to prevent integer overflow!" << std::endl;
      std::exit(1);
    }
  }

  std::cout << std::endl;
  std::cout << "All tests passed successfully!" << std::endl;

  return 0;
}
