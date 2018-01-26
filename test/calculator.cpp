///
/// @file   calculator.cpp
/// @brief  test program for calculator.hpp
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/calculator.hpp>

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <string>
#include <stdint.h>

#define STR1(s) #s
#define TOSTRING(s) STR1(s)

/// Test expressions
#define EXPR1 45345 + 0 + 0xdf234 - 1000 % 7
#define EXPR2 (0 + 0xdf234 - 1000) * 3 / 2 % 999
#define EXPR3 1 << 16
#define EXPR4 (0 + ~(0xdf234 & 1000) * 3) / -2
#define EXPR5 ((1 << 16) + (1 << 16)) >> 0X5
#define EXPR6 1+(((2+(3+(4+(5+6)* -7)/8))&127)<<1) *-3
#define EXPR7 100000000 + (1 << 16) + (1 << 16)
#define EXPR8 1-~1
#define EXPR9 1- ~1*0xfFa/( ((((8+(6|(4 *(2*(1)*3)*5)|7)+9)))))
#define EXPRa ((12|13)<<8)>>((1|127) %10&(31+7))
#define EXPRb ((((((((((5))))))  ))))- ((((((((( 6)))))))))

using namespace std;

template <typename T>
void compare(T result, const string& str)
{
  T r = calculator::eval<T>(str);
  cout << (r == result ? "Correct: " : "Error: ");
  cout << setw(50) << str << " = " << setw(10) << r;
  if (r != result)
  {
    cout << " != " << result;
    exit(1);
  }
  cout << endl;
}

int main()
{
  cout.setf(ios::left);

  compare(EXPR1, TOSTRING(EXPR1));
  compare(EXPR2, TOSTRING(EXPR2));
  compare(EXPR3, TOSTRING(EXPR3));
  compare(EXPR4, TOSTRING(EXPR4));
  compare(EXPR5, TOSTRING(EXPR5));
  compare(EXPR6, TOSTRING(EXPR6));
  compare(EXPR7, TOSTRING(EXPR7));
  compare(EXPR8, TOSTRING(EXPR8));
  compare(EXPR9, TOSTRING(EXPR9));
  compare(EXPRa, TOSTRING(EXPRa));
  compare(EXPRb, TOSTRING(EXPRb));

  compare(calculator::eval<int64_t>("1e18"), "1000000000000000000");
  compare(calculator::eval<int64_t>("3e18"), "3000000000000000000");
  compare(calculator::eval<int64_t>("10^0"), "1");
  compare(calculator::eval<int64_t>("10^1"), "10");
  compare(calculator::eval<int64_t>("37^2"), "1369");
  compare(calculator::eval<int64_t>("101^3"), "1030301");
  compare(calculator::eval<int64_t>("3^30"), "205891132094649");
  compare(calculator::eval<int64_t>("2^62-1"), "4611686018427387903");

  cout << "All tests passed successfully!" << endl;

  return 0;
}
