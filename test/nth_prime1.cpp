///
/// @file   nth_prime1.cpp
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
#include <stdexcept>

using namespace std;
using namespace primesieve;

void check(bool isCorrect)
{
  cout << "   " << (isCorrect ? "OK" : "ERROR") << "\n";
  if (!isCorrect)
    throw runtime_error("test failed!");
}

int main()
{
  try
  {
    uint64_t upperBound = (uint64_t) 1e5;
    primesieve::iterator it;
    uint64_t prime = it.next_prime();
    int64_t n;

    // nth_prime(n) forwards
    for (n = 0; prime < upperBound; prime = it.next_prime())
    {
      n++;
      uint64_t res = nth_prime(n);
      cout << "nth_prime(" << n << ") = " << res;
      check(res == prime);
    }

    prime = it.prev_prime();

    // nth_prime(-n, start) backwards
    for (n = 0; prime > 0; prime = it.prev_prime())
    {
      n--;
      uint64_t res = nth_prime(n, upperBound);
      cout << "nth_prime(" << n << ", " << upperBound << ") = " << res;
      check(res == prime);
    }

    cout << endl;
    cout << "All tests passed successfully!" << endl;
  }
  catch (exception& e)
  {
    cerr << endl << "primesieve error: " << e.what() << endl;
    return 1;
  }

  return 0;
}
