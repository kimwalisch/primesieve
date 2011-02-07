/*
 * main.cpp -- This file is part of ExpressionParser
 *
 * Copyright (C) 2011 Kim Walisch, <kim.walisch@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

/**
 * @file    main.cpp
 * @brief   This file contains 3 simple command-line programs used to
 *          test ExpressionParser (see ExpressionParser.h).
 * @author  Kim Walisch, <kim.walisch@gmail.com>
 * @version 1.0
 * @date    January 2011
 *
 * == Programs ==
 *
 * expr_correctness : tests ExpressionParser.h for correctness.
 * expr_benchmark   : tool used to benchmark ExpressionParser.
 * expr             : default program, evaluates arithmetic
 *                    expressions passed as command-line arguments and
 *                    prints the result.
 */

#if defined(EXPRESSIONPARSER_CORRECTNESS)

/**
 * expr_correctness, test ExpressionParser for correctness.
 * @brief A simple test program for ExpressionParser that evaluates
 *        several arithmetic test expressions and compares the
 *        results with the correct values calculated by the compiler.
 *
 * == How to compile ==
 *
 * GNU GCC:
 * $ g++ -DEXPRESSIONPARSER_CORRECTNESS main.cpp -o expr_correctness
 *
 * Microsoft Visual C++:
 * > cl /EHsc /D "EXPRESSIONPARSER_CORRECTNESS" main.cpp /Feexpr_correctness
 */

#include "ExpressionParser.h"

#include <iostream>
#include <iomanip>
#include <string>

/// Incremented for erroneous tests
int failed = 0;

/**
 * Evaluate the expression string and compare the result with
 * the correct value.
 */
template<class T>
void evalTest(T result, const std::string& str) {
  ExpressionParser<T> parser;
  parser.eval(str);
  std::cout.setf(std::ios::left);
  if (result == parser.getResult()) {
    std::cout << "CORRECT: ";
  } else {
    failed++;
    std::cout << "ERROR:   ";
  }
  std::cout << std::setw(43) << str << " = "
            << std::setw(10) << parser.getResult();
  if (result != parser.getResult())
    std::cout << " != " << result;
  std::cout << std::endl;
}

int main () {
  int expression = 0;
  std::cout << "Evaluating expressions using int type:" << std::endl;

  expression = 45345 + 0 + 0xdf234 - 1000 % 7;
  evalTest<int>(expression, "45345 + 0 + 0xdf234 - 1000 % 7");
  expression = (0 + 0xdf234 - 1000) * 3 / 2 % 999;
  evalTest<int>(expression, "(0 + 0xdf234 - 1000) * 3 / 2 % 999");
  expression = 1 << 16;
  evalTest<int>(expression, "2^2^2^2");
  expression = (0 + ~(0xdf234 & 1000) * 3) / -2;
  evalTest<int>(expression, "(0 + not (0xdf234 and 1000) * 3) / -2");
  expression = ((1 << 16) + (1 << 16)) >> 0X5;
  evalTest<int>(expression, "((2^16) + (1 SHL 16)) shr 0X5");
  expression = 1+(((2+(3+(4+(5+6)*-7)/8))&127)<<1)*-3;
  evalTest<int>(expression, "1+(((2+(3+(4+(5+6)*-7)/8))AND127)shl1)*-3");
  expression = 100000000 + (1 << 16) + (1 << 16);
  evalTest<int>(expression, "1e8 + 2^16 + 2 **16");
  expression = 1-~1;
  evalTest<int>(expression, "1-NOT1");
  expression = 1-~1*0xfFa/(((((8+(6|(4*(2*(1)*3)*5)|7)+9)))));
  evalTest<int>(expression, "1-NOT1*0xfFa/(8+(6or(4*(2*(1)*3)*5)OR7)+9)");
  expression = ((12|13)<<8)>>((1|127)%10&(31+7));
  evalTest<int>(expression, "((12or13)SHL8)SHR((1OR127)%10and(31+7))");
  expression = ((((((((((5))))))))))-(((((((((6)))))))));
  evalTest<int>(expression, "((((((((((5))))))))))-(((((((((6)))))))))");
  expression = -(+(-(+(-(-5)))))*-(+(-(+(-(-6)))));
  evalTest<int>(expression, "-(+(-(+(-(-5)))))*-(+(-(+(-(-6)))))");

  if (failed == 0) {
    std::cout << "All tests passed successfully!" << std::endl;
  } else {
    std::cerr << failed << " test(s) failed!" << std::endl;
    return 1;
  }
  return 0;
}

#elif defined(BENCHMARK_EXPRESSIONPARSER) && defined(_OPENMP)

/**
 * expr_benchmark, benchmark program for ExpressionParser.
 * @brief A multi-threaded (OpenMP) command-line program that repeats
 *        the evaluation of a single arithmetic expression and prints
 *        the sum of the results and the time elapsed when finished.
 *
 * == Usage ==
 *
 * Usage: expr_benchmark REPEAT EXPRESSION [-t <N>]
 * Repeat the expression evaluation as a benchmark for ExpressionParser
 * and print the sum of the results and the time elapsed.
 * Option: -t <N>, set the number of threads e.g. -t 4
 * Example: expr_benchmark 1e8 "5*(2^(9+7))/3+5*(1AND0xFf123)+(((1shl16)*3)%99)"
 *
 * == How to compile ==
 *
 * GNU GCC:
 * $ g++ -DBENCHMARK_EXPRESSIONPARSER -DNDEBUG -O2 -fopenmp main.cpp -o expr_benchmark
 *
 * Microsoft Visual C++:
 * > cl /EHsc /D "BENCHMARK_EXPRESSIONPARSER" /D "NDEBUG" /O2 /openmp main.cpp /Feexpr_benchmark
 */

#include "ExpressionParser.h"

#include <omp.h>
#include <iostream>
#include <string>
#include <cstring>
#include <ctime>

void help() {
  std::cerr << "Usage: expr_benchmark REPEAT EXPRESSION [-t <N>]" << std::endl
      << "Repeat the expression evaluation as a benchmark for ExpressionParser" << std::endl
      << "and print the sum of the results and the time elapsed." << std::endl
      << "Option: -t <N>, set the number of threads i.e. -t 4" << std::endl
      << "Example: expr_benchmark 1e8 "
      << "\"5*(2^(9+7))/3+5*(1AND0xFf123)+(((1shl16)*3)%99)\"" << std::endl;
}

void help2(const std::string& errorMessage, int threads = 0) {
  std::cerr << errorMessage;
  if (threads != 0)
    std::cerr << " " << threads;
  std::cerr << std::endl;
  std::cerr << "Try `expr_benchmark -h' for more information." << std::endl;
}

int main(int argc, char* argv[]) {
  ExpressionParser<int> parser;

  // check command-line arguments
  if (argc != 3 && !(argc == 5 && std::strlen(argv[3]) == 2 && (
      argv[3][0] == '-' ||
      argv[3][0] == '/') &&
      argv[3][1] == 't')) {
    help();
    return 1;
  }
  // check for syntax errors in EXPRESSION and REPEAT
  if (!parser.eval(argv[2]) ||
      !parser.eval(argv[1])) {
    help2(parser.getErrorMessage());
    return 1;
  }

  int repeat = parser.getResult();
  int threads = omp_get_max_threads();

  // handle the -t, /t <NUMBER OF THREADS> option
  if (argc == 5) {
    if (!parser.eval(argv[4])) {
      help2(parser.getErrorMessage());
      return 1;
    } else if (parser.getResult() > threads) {
      help2("Error: maximum number of threads for this CPU is", threads);
      return 1;
    } else if (parser.getResult() < 1) {
      help2("Error: minimum number of threads is 1");
      return 1;
    }
    threads = parser.getResult();
  }

  std::cout << "Number of threads: " << threads << std::endl;

  int sum = 0;
  int status = -1;
  int count = 0;
  double timing = omp_get_wtime();

  // repeat the expression evaluation as a benchmark for ExpressionParser
  #pragma omp parallel for private(parser) shared(count,status) \
      reduction(+:sum) num_threads(threads)
  for (int i = 0; i < repeat; i++) {
    parser.eval(argv[2]);
    sum += parser.getResult();
    // update global counter
    #pragma omp atomic
    count++;
    // calculate the current status in percents
    #pragma omp critical
    {
      int currentStatus = static_cast<int> (100.0 * (1.0 -
          static_cast<double> (repeat - count) /
          static_cast<double> (repeat)));
      if (currentStatus > status) {
        status = (currentStatus < 100) ?currentStatus :100;
        std::cout << "\rStatus: "<< status << "%"
            << std::flush;
      }
    }
  }
  timing = omp_get_wtime() - timing;

  std::cout << "\rStatus: 100%" << std::endl
            << "Sum of the results: " << sum << std::endl
            << "Time elapsed: " << timing << " sec" << std::endl;
  return 0;
}

#else

/**
 * expr, evaluate arithmetic expressions.
 * @brief A command-line program that evaluates integer arithmetic
 *        expressions passed as passed as command-line arguments and
 *        prints the results to the standard output.
 *
 * == Usage ==
 *
 * Usage: expr EXPRESSION...
 * Evaluate an integer arithmetic expression and print the result.
 * Example: expr "5 * (2^(9 + 7) and 127)"
 *
 * Supported operators:
 *
 *    or,  OR      Bitwise Inclusive OR
 *    xor, XOR     Bitwise Exclusive OR
 *    and, AND     Bitwise AND
 *    not, NOT     Unary complement
 *    shl, SHL     Shift Left
 *    shr, SHR     Shift Right
 *    +            Addition
 *    -            Subtraction
 *    *            Multiplication
 *    /            Division
 *    %            Modulo
 *    ^, **        Raise to power
 *    e, E         Scientific notation
 *
 * == How to compile ==
 *
 * GNU GCC:
 * $ g++ main.cpp -o expr
 *
 * Microsoft Visual C++:
 * > cl /EHsc main.cpp /Feexpr
 */

#include "ExpressionParser.h"
#include <iostream>

void help() {
  std::cerr << "Usage: expr EXPRESSION..." << std::endl
      << "Evaluate an integer arithmetic expression and print the result." << std::endl
      << "Example: expr \"5 * (2^(9 + 7) and 127)\"" << std::endl
      << std::endl
      << "Supported operators:" << std::endl
      << std::endl
      << "or,  OR      Bitwise Inclusive OR"  << std::endl
      << "xor, XOR     Bitwise Exclusive OR"  << std::endl
      << "and, AND     Bitwise AND"           << std::endl
      << "not, NOT     Unary complement"      << std::endl
      << "shl, SHL     Shift Left"            << std::endl
      << "shr, SHR     Shift Right"           << std::endl
      << "+            Addition"              << std::endl
      << "-            Subtraction"           << std::endl
      << "*            Multiplication"        << std::endl
      << "/            Division"              << std::endl
      << "%            Modulo"                << std::endl
      << "^, **        Raise to power"        << std::endl
      << "e, E         Scientific notation"   << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc <= 1) {
    help();
    return 1;
  }
  ExpressionParser<int> parser;
  for (int i = 1; i < argc; i++) {
    if (parser.eval(argv[i]) == true) {
      std::cout << parser.getResult() << std::endl;
    } else {
      std::cerr << parser.getErrorMessage() << std::endl
                << "Try `expr' (no arguments) for more information."
                << std::endl;
      return 1;
    }
  }
  return 0;
}

#endif
