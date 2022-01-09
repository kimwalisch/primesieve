///
/// @file   count_quintuplets.cpp
/// @brief  Count prime quintuplets inside [10^12, 10^12 + 10^9].
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve.hpp>

#include <stdint.h>
#include <iostream>
#include <cstdlib>

void check(bool OK)
{
  std::cout << "   " << (OK ? "OK" : "ERROR") << "\n";
  if (!OK)
    std::exit(1);
}

int main()
{
  uint64_t start = (uint64_t) 1e12;
  uint64_t stop = (uint64_t)(1e12 + 1e9);
  uint64_t count = primesieve::count_quintuplets(start, stop);
  std::cout << "Prime quintuplets inside [10^12, 10^12 + 10^9] = " << count;
  check(count == 1259);

  std::cout << std::endl;
  std::cout << "Test passed successfully!" << std::endl;

  return 0;
}
