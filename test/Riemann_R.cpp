///
/// @file   Riemann_R.cpp
/// @brief  Test the Riemann R function.
///
/// Copyright (C) 2024 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/RiemannR.hpp>

#include <stdint.h>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <limits>
#include <vector>

using std::max;
using std::size_t;
using namespace primesieve;

std::vector<uint64_t> RiemannR_table =
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
       3204941731601ll, // RiemannR(10^14)
      29844570495886ll, // RiemannR(10^15)
     279238341360977ll, // RiemannR(10^16)
    2623557157055978ll, // RiemannR(10^17)
   24739954284239494ll  // RiemannR(10^18)
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
  for (size_t i = 0; i < RiemannR_table.size(); i++)
  {
    // The accuracy of RiemannR(x) depends on
    // the width of the long double type.
    if (i >= std::numeric_limits<long double>::digits10)
      break;

    x *= 10;
    std::cout << "RiemannR(" << x << ") = " << (uint64_t) RiemannR((long double) x);
    check((uint64_t) RiemannR((long double) x) == RiemannR_table[i]);
  }

  x = 1;
  for (size_t i = 0; i < RiemannR_table.size(); i++)
  {
    // The accuracy of RiemannR(x) depends on
    // the width of the long double type.
    if (i >= std::numeric_limits<long double>::digits10)
      break;

    x *= 10;
    std::cout << "RiemannR_inverse(" << RiemannR_table[i] << ") = " << (uint64_t) RiemannR_inverse((long double) RiemannR_table[i]);
    check((uint64_t) RiemannR_inverse((long double) RiemannR_table[i]) < x &&
          (uint64_t) RiemannR_inverse((long double) RiemannR_table[i] + 1) >= x);
  }

  // Sanity checks for tiny values of RiemannR(x)
  for (x = 0; x < 10000; x++)
  {
    uint64_t rix = (uint64_t) RiemannR((long double) x);
    double logx = std::log(max((double) x, 2.0));

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
    double logx = std::log(max((double) x, 2.0));

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
