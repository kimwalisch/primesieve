///
/// @file   calculator.cpp
/// @brief  test program for calculator.hpp
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/calculator.hpp>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <string>

// If stdint.h defines the INT128_MAX macro we assume the
// int128_t and uint128_t types are well supported by
// the C++ standard library.
#include <stdint.h>

template <typename T>
std::string to_string(T n)
{
  return std::to_string(n);
}

// The compiler supports __int128_t but the C++ standard library
// does not, at least for GCC <= 15 and Clang <= 21.
// Hence we need to write some __int128_t helper functions.
#if defined(__SIZEOF_INT128__) && \
   !defined(INT128_MAX) && \
   !defined(_MSC_VER)

using int128_t = __int128_t;
using uint128_t = __uint128_t;

template <>
std::string to_string(uint128_t n)
{
  std::string str;

  while (n > 0)
  {
    str += '0' + n % 10;
    n /= 10;
  }

  if (str.empty())
    str = "0";

  std::reverse(str.begin(), str.end());

  return str;
}

template <>
std::string to_string(int128_t n)
{
  if (n >= 0)
    return to_string((uint128_t) n);
  else
  {
    // -n causes undefined behavior for n = INT128_MIN.
    // Hence we use the defined two's complement negation: ~n + 1.
    // Casting ~n to unsigned ensures the result of the addition
    // (2^127 for INT128_MIN) is safely stored in a uint128_t
    // without signed overflow.
    uint128_t abs_n = uint128_t(~n) + 1;
    return "-" + to_string(abs_n);
  }
}
#endif

template <typename T>
void check(const std::string& expr,
           const std::string& expected)
{
  T n = calculator::eval<T>(expr);
  std::string res = to_string(n);

  if (res == expected)
    std::cout << "Correct: " << expr << " = " << res << std::endl;
  else
  {
    std::cerr << "Error: " << expr << " = " << res << " != " << expected << std::endl;
    std::exit(1);
  }
}

template <typename T>
void check_exception(const std::string& expr)
{
  try {
    check<T>(expr, "error");
  }
  catch (calculator::error& e)
  {
    std::string errorMsg(e.what());
    std::size_t pos = errorMsg.find(": ");

    if (pos != std::string::npos)
      errorMsg = errorMsg.substr(pos + 2);

    std::cout << "Correct: " << errorMsg << std::endl;
  }
}

void trivial_tests()
{
  check<int>("0", "0");
  check<int>("1", "1");
  check<int>("12", "12");
  check<int>("123", "123");

  check<int>("-0", "0");
  check<int>("-1", "-1");
  check<int>("-12", "-12");
  check<int>("-123", "-123");

  check<int>("0x0", "0");
  check<int>("0x1", "1");
  check<int>("0xa", "10");
  check<int>("0xf", "15");
  check<int>("0x10", "16");
  check<int>("0xff", "255");

  check<int>("-0x0", "0");
  check<int>("-0x1", "-1");
  check<int>("-0xa", "-10");
  check<int>("-0xf", "-15");
  check<int>("-0x10", "-16");
  check<int>("-0xff", "-255");

  check<int>("(1)", "1");
  check<int>("2 * (-0xa + 17)", "14");
  check<int>("10* -(3+2)", "-50");
  check<int>("2^(2*5)", "1024");
  check<int>(" (2*5)^2", "100");
  check<int>("(((((1 )))+ 1))", "2");

  std::cout << std::endl;

  check_exception<int>("");
  check_exception<int>("     ");
  check_exception<int>("10 10");
  check_exception<int>("10a10");
  check_exception<int>("10.10");
  check_exception<int>("10'10");
  check_exception<int>("10\"10");
  check_exception<int>("()");
  check_exception<int>("10+(5-3");
  check_exception<int>("10+)");
  check_exception<int>("(((((10))))");
}

void arithmetic_expression_tests()
{
  #define STR1(s) #s
  #define STRINGIFY(s) STR1(s)

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

  check<int>(STRINGIFY(EXPR1), to_string(EXPR1));
  check<int>(STRINGIFY(EXPR2), to_string(EXPR2));
  check<int>(STRINGIFY(EXPR3), to_string(EXPR3));
  check<int>(STRINGIFY(EXPR4), to_string(EXPR4));
  check<int>(STRINGIFY(EXPR5), to_string(EXPR5));
  check<int>(STRINGIFY(EXPR6), to_string(EXPR6));
  check<int>(STRINGIFY(EXPR7), to_string(EXPR7));
  check<int>(STRINGIFY(EXPR8), to_string(EXPR8));
  check<int>(STRINGIFY(EXPR9), to_string(EXPR9));
  check<int>(STRINGIFY(EXPRa), to_string(EXPRa));
  check<int>(STRINGIFY(EXPRb), to_string(EXPRb));
}

void signed_integer_tests()
{
  check<int64_t>("300+(-200)", "100");
  check<int64_t>("300-(-200)", "500");
  check<int64_t>("1e18", "1000000000000000000");
  check<int64_t>("3e18", "3000000000000000000");
  check<int64_t>("7e-2", "0");
  check<int64_t>("10^0", "1");
  check<int64_t>("10^1", "10");
  check<int64_t>("37^2", "1369");
  check<int64_t>("101^3", "1030301");
  check<int64_t>("3^30", "205891132094649");
  check<int64_t>("2^62-1", "4611686018427387903");
  check<int64_t>("2^62-1+2^62", "9223372036854775807");
  check<int64_t>("-(2^62)-(2^62)", "-9223372036854775808");

  std::cout << std::endl;

  check<int64_t>("1^60", "1");
  check<int64_t>("(-1)^59", "-1");
  check<int64_t>("(-1)^60", "1");
  check<int64_t>("(-2)^3", "-8");
  check<int64_t>("(-2)^4", "16");
  check<int64_t>("3^3", "27");
  check<int64_t>("(-3)^3", "-27");
  check<int64_t>("(-3)^20", "3486784401");
  check<int64_t>("0^0", "1");

  std::cout << std::endl;

  check<int64_t>("(-1)^1", "-1");
  check<int64_t>("(-1)^-1", "-1");
  check<int64_t>("(-1)^-2", "1");
  check<int64_t>("(-1)^-3", "-1");
  check<int64_t>("(-1)^-4", "1");
  check<int64_t>("2^-1", "0");
  check<int64_t>("1000^-2", "0");
  check<int64_t>("100000000^-5", "0");
  check<int64_t>("(-2)^-1", "0");
  check<int64_t>("(-1000)^-2", "0");
  check<int64_t>("(-100000000)^-5", "0");

  std::cout << std::endl;

  check_exception<int64_t>("0^(-1)");
  check_exception<int64_t>("0xfffffffffffffffffff");
  check_exception<int64_t>("1000000000000000000000000000");
  check_exception<int64_t>("10^20");
  check_exception<int64_t>("123456789012345*1234567890");
  check_exception<int64_t>("9223372036854775700+200");
  check_exception<int64_t>("-9223372036854775700+(-200)");
  check_exception<int64_t>("-9223372036854775700-200");
  check_exception<int64_t>("9223372036854775700-(-200)");
  check_exception<int64_t>("-(-9223372036854775807-1)");

#if defined(__SIZEOF_INT128__) && \
   !defined(_MSC_VER)

  std::cout << std::endl;

  check<int128_t>("1e25", "10000000000000000000000000");
  check<int128_t>("3e25", "30000000000000000000000000");
  check<int128_t>("5^50", "88817841970012523233890533447265625");
  check<int128_t>("2^120-1", "1329227995784915872903807060280344575");
  check<int128_t>("2^126-1+2^126", "170141183460469231731687303715884105727");
  check<int128_t>("-(2^126)-(2^126)", "-170141183460469231731687303715884105728");

  std::cout << std::endl;

  check_exception<int128_t>("0xfffffffffffffffffffffffffffffffff");
  check_exception<int128_t>("10000000000000000000000000000000000000000");
  check_exception<int128_t>("10^40");
  check_exception<int128_t>("170141183460469231731687303715884105700*2");
  check_exception<int128_t>("170141183460469231731687303715884105700+200");
  check_exception<int128_t>("-170141183460469231731687303715884105700+(-200)");
  check_exception<int128_t>("-170141183460469231731687303715884105700-200");
  check_exception<int128_t>("170141183460469231731687303715884105700-(-200)");
  check_exception<int128_t>("-(-170141183460469231731687303715884105727-1)");

#endif
}

void unsigned_integer_tests()
{
  check<uint64_t>("300-200", "100");
  check<uint64_t>("1e19", "10000000000000000000");
  check<uint64_t>("11e18", "11000000000000000000");
  check<uint64_t>("10^0", "1");
  check<uint64_t>("10^1", "10");
  check<uint64_t>("37^2", "1369");
  check<uint64_t>("101^3", "1030301");
  check<uint64_t>("3^30", "205891132094649");
  check<uint64_t>("2^63-1", "9223372036854775807");
  check<uint64_t>("2^63-1+2^63", "18446744073709551615");

  std::cout << std::endl;

  check_exception<uint64_t>("0xfffffffffffffffffff");
  check_exception<uint64_t>("1000000000000000000000000000");
  check_exception<uint64_t>("10^20");
  check_exception<uint64_t>("123456789012345*1234567890");
  check_exception<uint64_t>("18446744073709551516+200");
  check_exception<uint64_t>("2-3");
  check_exception<uint64_t>("-100+200");

#if defined(__SIZEOF_INT128__) && \
   !defined(_MSC_VER)

  std::cout << std::endl;

  check<uint128_t>("1e25", "10000000000000000000000000");
  check<uint128_t>("3e25", "30000000000000000000000000");
  check<uint128_t>("5^50", "88817841970012523233890533447265625");
  check<uint128_t>("2^120-1", "1329227995784915872903807060280344575");
  check<uint128_t>("2^127-1+2^127", "340282366920938463463374607431768211455");

  std::cout << std::endl;

  check_exception<uint128_t>("0xfffffffffffffffffffffffffffffffff");
  check_exception<uint128_t>("10000000000000000000000000000000000000000");
  check_exception<uint128_t>("10^40");
  check_exception<uint128_t>("340282366920938463463374607431768211356*2");
  check_exception<uint128_t>("340282366920938463463374607431768211356+200");
  check_exception<uint128_t>("340282366920938463463374607431768211356-340282366920938463463374607431768211357");
  check_exception<uint128_t>("100-(-100)");

#endif
}

#define TEST_OP(TYPE, OP, OP_STR) \
{ \
  if (j == 0 && ( \
      std::string(OP_STR) == "/" || \
      std::string(OP_STR) == "%")) \
    continue; \
  if (i OP j >= std::numeric_limits<TYPE>::min() && \
      i OP j <= std::numeric_limits<TYPE>::max()) \
  { \
    std::string expr = std::to_string(i) + OP_STR + std::to_string(j); \
    int res = calculator::eval<TYPE>(expr); \
    if (i OP j != res) \
    { \
      std::cerr << "Error: " << i << " " << OP_STR << " " << j << " = " << i OP j << " != " << res << std::endl; \
      std::exit(1); \
    } \
  } \
}

// This test tests invalid arithmetic expressions that cause
// e.g. integer overflow or underflow. If an arithmetic
// expression is invalid, then our code must detect it and
// throw a calculator::error exception.
#define TEST_OP_OVERFLOW(TYPE, OP, OP_STR) \
{ \
  if (j == 0 && ( \
      std::string(OP_STR) == "/" || \
      std::string(OP_STR) == "%")) \
    continue; \
  std::string expr; \
  int res; \
  try { \
    expr = std::to_string(i) + OP_STR + std::to_string(j); \
    res = calculator::eval<TYPE>(expr); \
  } \
  catch (calculator::error&) { \
    count_exceptions += 1; \
    if (count_exceptions > max_exceptions) break; \
    res = std::numeric_limits<int>::min(); \
  } \
  if (i OP j >= std::numeric_limits<TYPE>::min() && \
      i OP j <= std::numeric_limits<TYPE>::max()) \
  { \
    if (i OP j != res) \
    { \
      std::cerr << "Error: " << i << " " << OP_STR << " " << j << " = " << i OP j << " != " << res << std::endl; \
      std::exit(1); \
    } \
  } \
  else if (res != std::numeric_limits<int>::min()) \
  { \
      std::cerr << "Error: failed to detect invalid " << #TYPE << " expression: '" << expr << "'" << std::endl; \
      std::exit(1); \
  } \
}

void int8_tests()
{
  std::cout << "Starting int8_t tests..." << std::endl;
  int int8_min = std::numeric_limits<int8_t>::min();
  int int8_max = std::numeric_limits<int8_t>::max();

  for (int i = int8_min + 1; i <= int8_max; i++)
    for (int j = int8_min + 1; j <= int8_max; j++)
    {
      TEST_OP(int8_t, +, "+")
      TEST_OP(int8_t, -, "-")
      TEST_OP(int8_t, *, "*")
      TEST_OP(int8_t, /, "/")
      TEST_OP(int8_t, %, "%")
    }

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dist(int8_min + 1, int8_max);

  // C++ Exceptions are slow so we limit their number
  int max_exceptions = 5000;
  int count_exceptions = 0;

  for (int t = 0; t < 50000; t++)
  {
    int i = dist(gen);
    int j = dist(gen);

    TEST_OP_OVERFLOW(int8_t, +, "+")
    TEST_OP_OVERFLOW(int8_t, -, "-")
    TEST_OP_OVERFLOW(int8_t, *, "*")
    TEST_OP_OVERFLOW(int8_t, /, "/")
    TEST_OP_OVERFLOW(int8_t, %, "%")
  }

  std::cout << "Successfully completed int8_t tests!" << std::endl;
}

void uint8_tests()
{
  std::cout << "Starting uint8_t tests..." << std::endl;
  int uint8_min = std::numeric_limits<uint8_t>::min();
  int uint8_max = std::numeric_limits<uint8_t>::max();

  for (int i = uint8_min; i <= uint8_max; i++)
    for (int j = uint8_min; j <= uint8_max; j++)
    {
      TEST_OP(uint8_t, +, "+")
      TEST_OP(uint8_t, -, "-")
      TEST_OP(uint8_t, *, "*")
      TEST_OP(uint8_t, /, "/")
      TEST_OP(uint8_t, %, "%")
    }

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dist(uint8_min, uint8_max);

  // C++ Exceptions are slow so we limit their number
  int max_exceptions = 5000;
  int count_exceptions = 0;

  for (int t = 0; t < 50000; t++)
  {
    int i = dist(gen);
    int j = dist(gen);

    TEST_OP_OVERFLOW(uint8_t, +, "+")
    TEST_OP_OVERFLOW(uint8_t, -, "-")
    TEST_OP_OVERFLOW(uint8_t, *, "*")
    TEST_OP_OVERFLOW(uint8_t, /, "/")
    TEST_OP_OVERFLOW(uint8_t, %, "%")
  }

  std::cout << "Successfully completed uint8_t tests!" << std::endl;
}

int main()
{
  std::cout << std::endl;
  std::cout << "=== Trivial tests ===" << std::endl;
  std::cout << std::endl;

  trivial_tests();

  std::cout << std::endl;
  std::cout << "=== Arithmetic expression tests ===" << std::endl;
  std::cout << std::endl;

  arithmetic_expression_tests();

  std::cout << std::endl;
  std::cout << "=== Signed integer tests ===" << std::endl;
  std::cout << std::endl;

  signed_integer_tests();

  std::cout << std::endl;
  std::cout << "=== Unsigned integer tests ===" << std::endl;
  std::cout << std::endl;

  unsigned_integer_tests();

  std::cout << std::endl;
  std::cout << "=== 8-bit integer tests ===" << std::endl;
  std::cout << std::endl;

  int8_tests();
  uint8_tests();

  std::cout << std::endl;
  std::cout << "All tests passed successfully!" << std::endl;

  return 0;
}
