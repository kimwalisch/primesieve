/*
 * main.cpp -- This file is part of primesieve
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
 * @file  main.cpp
 * @brief Command-line version of primesieve, multi-threaded (OpenMP),
 *        compiles on many platforms.
 *
 * primesieve is a highly optimized implementation of the sieve of
 * Eratosthenes that generates prime numbers and prime k-tuplets (twin
 * primes, prime triplets, ...) up to 2^64 maximum.
 */

#include "../expr/ExpressionParser.h"
#include "../soe/ParallelPrimeSieve.h"
#include "../soe/pmath.h"

/// declared in test.cpp
void test();

#include <stdint.h>
#include <exception>
#include <iostream>
#include <iomanip>    /* std::setw(int) */
#include <cstdlib>    /* std::exit(int) */
#include <string>
#include <cctype>     /* std::tolower(int) */

namespace {
  // Unfortunately there is no easy way to get the CPU L1 and L2 cache
  // size, these values are close for most x86-64 CPUs in 2011
  const uint32_t L1_CACHE_SIZE = 64;
  const uint32_t L2_CACHE_SIZE = 512;
  const uint64_t L2_THRESHOLD = ipow(10, 13);

  bool quietMode = false;
  bool showExpressionResults = false;

  uint64_t numbers[2] = {0, 0};  /* start and stop number for sieving */
  uint32_t sieveSize = 0;        /* sieve size in KiloBytes */
  uint32_t flags = 0;            /* settings */

  int maxThreads = ParallelPrimeSieve::getMaxThreads();
  int threads = -1;

  std::string primes[7] = { "Prime numbers", "Twin primes", "Prime triplets",
    "Prime quadruplets", "Prime quintuplets", "Prime sextuplets",
    "Prime septuplets" };
}

void version() {
  std::cout << "primesieve 2.0, <http://primesieve.googlecode.com>" << std::endl
            << "Copyright (C) 2011 Kim Walisch" << std::endl
            << "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>." << std::endl
            << "This is free software: you are free to change and redistribute it." << std::endl
            << "There is NO WARRANTY, to the extent permitted by law." << std::endl;
  std::exit(EXIT_SUCCESS);
}

void help() {
  std::cout << "Usage: primesieve START STOP [OPTION]..." << std::endl
            << "Use the sieve of Eratosthenes to find the prime numbers and prime" << std::endl
            << "k-tuplets between START and STOP < 2^64" << std::endl
            << std::endl
            << "Examples:" << std::endl
            << "  primesieve 1 10000000 -p1" << std::endl
            << "  primesieve 1 1e11 -s 32" << std::endl
            << "  primesieve 1e18 1e18+2**32 -c1234567" << std::endl
            << std::endl
            << "Options:" << std::endl
            << "  -c[N]        Count prime numbers and/or prime k-tuplets," << std::endl
            << "               e.g -c1 count prime numbers, -c3 count prime triplets, ..." << std::endl
            << "               N >= 1 and N <= 7" << std::endl
            << "  -p[N]        Print prime numbers or prime k-tuplets," << std::endl
            << "               e.g -p1 print prime numbers, -p2 print twin primes, ..." << std::endl
            << "               N >= 1 and N <= 7" << std::endl
            << "  -q           Quiet mode, print less output" << std::endl
            << "  -s <SIZE>    Set the sieve size in KiloBytes, e.g. -s 256," << std::endl
            << "               set SIZE to your CPU's L1/L2 cache size for best performance" << std::endl
            << "               SIZE >= 1 and SIZE <= 8192" << std::endl
            << "  -t <THREADS> Set the number of threads for sieving, e.g. -t 4," << std::endl
            << "               THREADS >= 1 and THREADS <= " << maxThreads << " (CPU max threads)" << std::endl
            << "  -test        Run various sieving tests and exit" << std::endl
            << "  -v           Print version and license information and exit" << std::endl;
  std::exit(EXIT_SUCCESS);
}

bool isDigits(const std::string &str) {
  if (str.length() == 0)
    return false;
  return str.find_first_not_of("0123456789") == std::string::npos;
}

/**
 * Process the command-line options.
 * @see help(void)
 */
void processOptions(int argc, char* argv[]) {
  if (argc < 2 || argc > 20)
    help();
  ExpressionParser<uint64_t> parser;
  int i = 1;

  // process the START and STOP numbers
  if (argc > 2)
    for (; i <= 2; i++) {
      if (!parser.eval(argv[i])) {
        std::cerr << "Error: \"" << argv[i]  << "\" is not a valid arithmetic expression" << std::endl
                  << "Try `primesieve -help' for more information." << std::endl;
        std::exit(EXIT_FAILURE);
      }
      numbers[i-1] = parser.getResult();
      if (!isDigits(argv[i]))
        showExpressionResults = true;
    }

  // process the options ([OPTION]...)
  for (; i < argc; i++) {
    char* c = argv[i];
    if (*c != '-' && *c != '/')
      help();
    switch (std::tolower(*(++c)++)) {
      case 'c': do {
                  if (*c < '1' || *c > '7')
                    help();
                  flags |= COUNT_PRIMES << (*c - '1');
                } while (*(++c) != 0);
                break;
      case 'p': if (*c < '1' || *c > '7')
                  help();
                flags |= PRINT_PRIMES << (*c - '1');
                quietMode = true;
                break;
      case 'q': quietMode = true;
                break;
      case 's': if (++i >= argc || !parser.eval(argv[i]))
                  help();
                sieveSize = static_cast<uint32_t> (parser.getResult());
                sieveSize = nextHighestPowerOf2(sieveSize);
                break;
      case 't': if (std::string("test").compare(++argv[i]) == 0) {
                  test();
                  std::exit(EXIT_SUCCESS);
                }
                if (++i >= argc || !parser.eval(argv[i]))
                  help();
                threads = static_cast<int> (parser.getResult());
                break;
      case 'v': version();
      default : help();
    }
  }
}

/**
 * Process the command-line options and start sieving.
 */
int main(int argc, char* argv[]) {
  processOptions(argc, argv);
  std::cout.setf(std::ios::left);

  // display expression results
  if (!quietMode && showExpressionResults)
    std::cout << std::setw(10) << "START" << " = " << numbers[0] << std::endl
              << std::setw(10) << "STOP"  << " = " << numbers[1]  << std::endl;

  // set default settings
  if ((flags & COUNT_FLAGS) == 0 && (flags & PRINT_FLAGS) == 0)
    flags |= COUNT_PRIMES;
  if (!quietMode && (flags & PRINT_FLAGS) == 0)
    flags |= PRINT_STATUS;
  if (sieveSize == 0)
    sieveSize = (numbers[1] < L2_THRESHOLD) ?L1_CACHE_SIZE :L2_CACHE_SIZE;

  if (!quietMode)
    std::cout << std::setw(10) << "Sieve size" << " = " << sieveSize
              << " KiloBytes" << std::endl;
  try {
    // init primeSieve
    ParallelPrimeSieve primeSieve;
    primeSieve.setStartNumber(numbers[0]);
    primeSieve.setStopNumber(numbers[1]);
    primeSieve.setSieveSize(sieveSize);
    primeSieve.setFlags(flags);

    if (threads == -1)
      threads = primeSieve.getIdealThreadCount();
    if (!quietMode)
      std::cout << std::setw(10) << "Threads" << " = " << threads << std::endl;

    // start sieving primes
    primeSieve.sieve(threads);

    // print new line
    if ((flags & PRINT_STATUS) || (
        (flags & PRINT_FLAGS) &&
        (flags & COUNT_FLAGS) ))
      std::cout << std::endl;

    // get max output width
    std::size_t width = (quietMode) ?0 :12;
    for (int i = 0; i < primeSieve.COUNTS_SIZE; i++)
      if ((flags & (COUNT_PRIMES << i)) && width < primes[i].length())
        width = primes[i].length();

    // print prime count results
    for (int i = 0; i < primeSieve.COUNTS_SIZE; i++)
      if (flags & (COUNT_PRIMES << i))
        std::cout << std::setw(static_cast<int> (width)) << primes[i] << " : "
                  << primeSieve.getCounts(i) << std::endl;

    // print time elapsed
    if (!quietMode)
      std::cout << std::setw(static_cast<int> (width)) << "Time elapsed" << " : "
                << primeSieve.getTimeElapsed() << " sec" << std::endl;
  }
  catch (std::exception& ex) {
    std::cerr << "Error: " << ex.what() << std::endl
              << "Try `primesieve -help' for more information." << std::endl;
    std::exit(EXIT_FAILURE);
  }
  return 0;
}
