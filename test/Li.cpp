///
/// @file   Li.cpp
/// @brief  Test the offset logarithmic integral function.
///         Li(x) = li(x) - li(2)
///
/// Copyright (C) 2024 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/nthPrimeApprox.hpp>

#include <stdint.h>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <vector>

using std::max;
using std::size_t;
using namespace primesieve;

std::vector<uint64_t> Li_table =
{
                     5, // Li(10^1)
                    29, // Li(10^2)
                   176, // Li(10^3)
                  1245, // Li(10^4)
                  9628, // Li(10^5)
                 78626, // Li(10^6)
                664917, // Li(10^7)
               5762208, // Li(10^8)
              50849233, // Li(10^9)
             455055613, // Li(10^10)
          4118066399ll, // Li(10^11)
         37607950279ll, // Li(10^12)
        346065645809ll, // Li(10^13)
       3204942065690ll, // Li(10^14)
      29844571475286ll  // Li(10^15)
};

void check(bool OK)
{
  std::cout << "   " << (OK ? "OK" : "ERROR") << "\n";
  if (!OK)
    std::exit(1);
}

int main()
{
  uint64_t x = 1;
  for (size_t i = 0; i < Li_table.size(); i++)
  {
    x *= 10;
    std::cout << "Li(" << x << ") = " << Li(x);
    check(Li(x) == Li_table[i]);
  }

  x = 1;
  for (size_t i = 0; i < Li_table.size(); i++)
  {
    x *= 10;
    std::cout << "Li_inverse(" << Li_table[i] << ") = " << Li_inverse(Li_table[i]);
    check(Li_inverse(Li_table[i]) <= x &&
          Li_inverse(Li_table[i] + 1) > x);
  }

  // Sanity checks for small values of Li(x)
  for (x = 0; x < 300000; x++)
  {
    uint64_t lix = Li(x);
    double logx = std::log(max((double) x, 2.0));

    if ((x >= 11 && lix < x / logx) ||
        (x >= 2  && lix > x * logx))
    {
      std::cout << "Li(" << x << ") = " << lix << "   ERROR" << std::endl;
      std::exit(1);
    }
  }

  // Sanity checks for small values of Li_inverse(x)
  for (x = 2; x < 30000; x++)
  {
    uint64_t res = Li_inverse(x);
    double logx = std::log((double) x);

    if (res < x ||
        (x >= 4 && res > x * logx * logx))
    {
      std::cout << "Li_inverse(" << x << ") = " << res << "   ERROR" << std::endl;
      std::exit(1);
    }
  }

  {
    uint64_t x = std::numeric_limits<uint64_t>::max() / 10;
    uint64_t res = Li_inverse(x);
    if (res != std::numeric_limits<uint64_t>::max())
    {
      std::cout << "Li_inverse(" << x << ") != UINT64_MAX, failed to prevent integer overflow!" << std::endl;
      std::exit(1);
    }
  }

  std::cout << std::endl;
  std::cout << "All tests passed successfully!" << std::endl;

  return 0;
}
