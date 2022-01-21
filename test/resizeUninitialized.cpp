///
/// @file   resizeUninitialized.cpp
/// @brief  Test resizeUninitialized() which resizes a std::vector
///         without default initialization.
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/resizeUninitialized.hpp>

#include <stdint.h>
#include <iostream>
#include <vector>
#include <cstdlib>

void check(bool OK)
{
  std::cout << "   " << (OK ? "OK" : "ERROR") << "\n";
  if (!OK)
    std::exit(1);
}

int main()
{
  std::size_t size = 100000;
  uint64_t val = (1ull << 60) - 3;

  {
    std::vector<uint64_t> vect;
    vect.resize(size, val);

    vect.clear();
    resizeUninitialized(vect, size);

    std::cout << "vect.size() = " << vect.size();
    check(vect.size() == size);

    std::cout << "vect.capacity() = " << vect.capacity();
    check(vect.capacity() == size);

    for (std::size_t i = 0; i < size; i += 37)
    {
      std::cout << "vect[" << i << "] = " << vect[i];
      check(vect[i] == val);
    }

    std::size_t oldSize = size;
    size /= 67;
    resizeUninitialized(vect, size);

    std::cout << "vect.size() = " << vect.size();
    check(vect.size() == size);

    std::cout << "vect.capacity() = " << vect.capacity();
    check(vect.capacity() == oldSize);

    for (std::size_t i = 0; i < size; i += 37)
    {
      std::cout << "vect[" << i << "] = " << vect[i];
      check(vect[i] == val);
    }
  }

  std::cout << std::endl;
  std::cout << "All tests passed successfully!" << std::endl;

  return 0;
}
