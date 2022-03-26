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

#include <cstdint>
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

    // After resizeUninitialized() the old vector
    // content must still be the same.
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

    // After resizeUninitialized() to a smaller size
    // there must be no reallocation. The capacity
    // must still be the same as before.
    std::size_t newSize = size / 67;
    resizeUninitialized(vect, newSize);

    std::cout << "vect.size() = " << vect.size();
    check(vect.size() == newSize);

    std::cout << "vect.capacity() = " << vect.capacity();
    check(vect.capacity() == size);

    for (std::size_t i = 0; i < newSize; i += 37)
    {
      std::cout << "vect[" << i << "] = " << vect[i];
      check(vect[i] == val);
    }

    // Test that reallocation works correctly.
    // First print the current vector address.
    uintptr_t address1 = (uintptr_t) vect.data();
    std::cout << "1st vector allocation: " << address1 << std::endl;

    // There must be no reallocation here.
    vect.clear();
    resizeUninitialized(vect, size);
    uintptr_t address2 = (uintptr_t) vect.data();
    std::cout << "1st vector allocation: " << address2 << std::endl;

    if (address1 != address2)
    {
      std::cout << "address2 = " << address2;
      check(address2 == address1);
      std::exit(1);
    }

    // This causes a reallocation, the old vector
    // content must be copied into the new vector.
    resizeUninitialized(vect, size * 50);
    uintptr_t address3 = (uintptr_t) vect.data();
    std::cout << "2nd vector allocation: " << address3 << std::endl;

    std::cout << "vect.size() = " << vect.size();
    check(vect.size() == size * 50);
    std::cout << "vect.capacity() = " << vect.capacity();
    check(vect.capacity() == size * 50);

    for (std::size_t i = 0; i < size; i++)
    {
      if (vect[i] != val)
      {
        std::cout << "vect[" << i << "] = " << vect[i];
        check(vect[i] == val);
        std::exit(1);
      }
    }
  }

  std::cout << std::endl;
  std::cout << "All tests passed successfully!" << std::endl;

  return 0;
}
