///
/// @file   count_twins.cpp
/// @brief  Count prime triplets inside [10^12, 10^12 + 10^9].
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve.hpp>

#include <stdint.h>
#include <iostream>
#include <stdexcept>

using namespace std;

void check(bool isCorrect)
{
  cout << "   " << (isCorrect ? "OK" : "ERROR") << "\n";
  if (!isCorrect)
    throw runtime_error("Test failed!");
}

int main()
{
  uint64_t count = primesieve::count_triplets((uint64_t) 1e12, (uint64_t)(1e12 + 1e9));
  cout << "Prime triplets inside [10^12, 10^12 + 10^9] = " << count;
  check(count == 271316);

  cout << endl;
  cout << "Test passed successfully!" << endl;

  return 0;
}
