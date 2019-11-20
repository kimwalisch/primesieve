///
/// @file   number_of_bits.cpp
/// @brief  Test numberOfBits(x) function.
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/pmath.hpp>

#include <stdint.h>
#include <iostream>

using namespace std;

void check(bool OK)
{
  cout << "   " << (OK ? "OK" : "ERROR") << "\n";
  if (!OK)
    exit(1);
}

int main()
{
  int8_t i8 = 0;
  cout << "numberOfBits(int8_t) = " << (int) numberOfBits(i8);
  check(numberOfBits(i8) == 8);

  uint8_t ui8 = 0;
  cout << "numberOfBits(uint8_t) = " << (int) numberOfBits(ui8);
  check(numberOfBits(ui8) == 8);

  int16_t i16 = 0;
  cout << "numberOfBits(int16_t) = " << numberOfBits(i16);
  check(numberOfBits(i16) == 16);

  uint16_t ui16 = 0;
  cout << "numberOfBits(uint16_t) = " << numberOfBits(ui16);
  check(numberOfBits(ui16) == 16);

  int32_t i32 = 0;
  cout << "numberOfBits(int32_t) = " << numberOfBits(i32);
  check(numberOfBits(i32) == 32);

  uint32_t ui32 = 0;
  cout << "numberOfBits(uint32_t) = " << numberOfBits(ui32);
  check(numberOfBits(ui32) == 32);

  int64_t i64 = 0;
  cout << "numberOfBits(int64_t) = " << numberOfBits(i64);
  check(numberOfBits(i64) == 64);

  uint64_t ui64 = 0;
  cout << "numberOfBits(uint64_t) = " << numberOfBits(ui64);
  check(numberOfBits(ui64) == 64);

  cout << endl;
  cout << "All tests passed successfully!" << endl;

  return 0;
}
