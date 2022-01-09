///
/// @file   number_of_bits.cpp
/// @brief  Test numberOfBits<T>() function.
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/pmath.hpp>

#include <stdint.h>
#include <iostream>

void check(bool OK)
{
  std::cout << "   " << (OK ? "OK" : "ERROR") << "\n";
  if (!OK)
    std::exit(1);
}

int main()
{
  std::cout << "numberOfBits<int8_t>() = " << (int) numberOfBits<int8_t>();
  check(numberOfBits<int8_t>() == 8);

  std::cout << "numberOfBits<uint8_t>() = " << (int) numberOfBits<uint8_t>();
  check(numberOfBits<uint8_t>() == 8);

  std::cout << "numberOfBits<int16_t>() = " << numberOfBits<int16_t>();
  check(numberOfBits<int16_t>() == 16);

  std::cout << "numberOfBits<uint16_t>() = " << numberOfBits<uint16_t>();
  check(numberOfBits<uint16_t>() == 16);

  std::cout << "numberOfBits<int32_t>() = " << numberOfBits<int32_t>();
  check(numberOfBits<int32_t>() == 32);

  std::cout << "numberOfBits<uint32_t>() = " << numberOfBits<uint32_t>();
  check(numberOfBits<uint32_t>() == 32);

  std::cout << "numberOfBits<int64_t>() = " << numberOfBits<int64_t>();
  check(numberOfBits<int64_t>() == 64);

  std::cout << "numberOfBits<uint64_t>() = " << numberOfBits<uint64_t>();
  check(numberOfBits<uint64_t>() == 64);

  std::cout << std::endl;
  std::cout << "All tests passed successfully!" << std::endl;

  return 0;
}
