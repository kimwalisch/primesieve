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
 * Eratosthenes that finds prime numbers and prime k-tuplets (twin
 * primes, prime triplets, ...) up to 2^64 maximum.
 */

#include "../expr/ExpressionParser.h"
#include "../soe/ParallelPrimeSieve.h"
#include "../soe/pmath.h"
#include "../soe/cpuid.h"

/// declared in test.cpp
void test();

#include <stdint.h>
#include <exception>
#include <iostream>
#include <iomanip>    /* std::setw(int) */
#include <cstdlib>    /* std::exit(int) */
#include <string>
#include <cstring>    /* std::strlen(const char*) */
#include <cctype>     /* std::tolower(int) */
#include <sstream>

namespace {
  // Unfortunately there is no easy way to get the CPU L1 and L2 cache
  // size, these values are close for most x86-64 CPUs in 2011
  const uint32_t L1_CACHE_SIZE = 64;
  const uint32_t L2_CACHE_SIZE = 512;
  const uint64_t L2_THRESHOLD = ipow(10, 11) * 5;

  bool quietMode = false;
  bool showExpressionResults = false;

  uint64_t start = 0;         /* lower bound for sieving */
  uint64_t stop = 0;          /* upper bound for sieving */
  uint32_t sieveSize = 0;     /* sieve size in KiloBytes */
  uint32_t flags = 0;         /* settings */

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
            << "  -c[N]         Count prime numbers and/or prime k-tuplets," << std::endl
            << "                e.g -c1 count prime numbers, -c3 count prime triplets, ..." << std::endl
            << "                N >= 1 and N <= 7" << std::endl
            << "  -p[N]         Print prime numbers or prime k-tuplets," << std::endl
            << "                e.g -p1 print prime numbers, -p2 print twin primes, ..." << std::endl
            << "                N >= 1 and N <= 7" << std::endl
            << "  -q            Quiet mode, print less output" << std::endl
            << "  -s <SIZE>     Set the sieve size in KiloBytes, e.g. -s 256," << std::endl
            << "                set SIZE to your CPU's L1/L2 cache size for best performance" << std::endl
            << "                SIZE >= 1 and SIZE <= 8192" << std::endl
            << "  -t <THREADS>  Set the number of threads for sieving, e.g. -t 4," << std::endl
            << "                THREADS >= 1 and THREADS <= " << maxThreads << " (CPU max threads)" << std::endl
            << "  -test         Run various sieving tests (single-threaded) and exit" << std::endl
            << "  -v            Print version and license information and exit" << std::endl;
  std::exit(EXIT_SUCCESS);
}

void error(const std::string &str1, const std::string &str2 = "") {
  std::cerr << str1 << str2 << std::endl
            << "Try `primesieve -help' for more information." << std::endl;
  std::exit(EXIT_FAILURE);
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

  // process the START and STOP number
  if (argc > 2) {
    if (!isDigits(argv[i]))
      showExpressionResults = true;
    if (!parser.eval(argv[i++]))
      error("START is not a valid expression: ", parser.getErrorMessage(false));
    start = parser.getResult();
    if (!isDigits(argv[i]))
      showExpressionResults = true;
    if (!parser.eval(argv[i++]))
      error("STOP is not a valid expression: ", parser.getErrorMessage(false));
    stop = parser.getResult();
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
      case 's': if (!parser.eval(argv[++i]))
                  help();
                sieveSize = static_cast<uint32_t> (parser.getResult());
                if (sieveSize < 1 || sieveSize > 8192)
                  error("Sieve size must be >= 1 and <= 8192 KiloBytes");
                sieveSize = nextHighestPowerOf2(sieveSize);
                break;
      case 't': if (std::string("test").compare(++argv[i]) == 0) {
                  test();
                  std::exit(EXIT_SUCCESS);
                }
                if (!parser.eval(argv[++i]))
                  help();
                threads = static_cast<int> (parser.getResult());
                if (threads < 1 || threads > maxThreads) {
                  std::ostringstream message;
                  message << "Use a number of threads >= 1 and <= "
                          << maxThreads
                          << " for this CPU";
                  error(message.str());
                }
                break;
      case 'v': version();
      default : help();
    }
  }
  if ((flags & PRINT_FLAGS) && threads > 1)
    error("Printing is only allowed using a single thread");
}

/**
 * Process the command-line options and start sieving.
 */
int main(int argc, char* argv[]) {
  processOptions(argc, argv);
  int width = static_cast<int> (std::strlen("Sieve size"));
  std::cout.setf(std::ios::left);

  // display expression results
  if (!quietMode && showExpressionResults)
    std::cout << std::setw(width) << "START" << " = " << start << std::endl
              << std::setw(width) << "STOP"  << " = " << stop  << std::endl;
  if (start > stop)
    error("STOP must be >= START");
  if (stop >= UINT64_MAX - UINT32_MAX * UINT64_C(10))
    error("STOP must be < (2^64-1) - (2^32-1) * 10");

  // set default settings
  if ((flags & COUNT_FLAGS) == 0 && (flags & PRINT_FLAGS) == 0)
    flags |= COUNT_PRIMES;
  if ((flags & PRINT_FLAGS) == 0)
    flags |= PRINT_STATUS;
  if (sieveSize == 0)
    sieveSize = (stop < L2_THRESHOLD) ?L1_CACHE_SIZE :L2_CACHE_SIZE;

  if (!quietMode)
    std::cout << std::setw(width) << "Sieve size" << " = " << sieveSize
              << " KiloBytes" << std::endl;
  try {
    ParallelPrimeSieve primeSieve;
    primeSieve.setStartNumber(start);
    primeSieve.setStopNumber(stop);
    primeSieve.setSieveSize(sieveSize);
    primeSieve.setFlags(flags);

    if (threads == -1)
      threads = primeSieve.getIdealThreadCount();
    if (!quietMode)
      std::cout << std::setw(width) << "threads" << " = " << threads << std::endl;

    // start sieving primes
    primeSieve.sieve(threads);

    // get max output string length
    width = static_cast<int> (std::strlen("Time elapsed"));
    for (int i = 0; i < primeSieve.COUNTS_SIZE; i++) {
      if (flags & (COUNT_PRIMES << i)) {
        int length = static_cast<int> (primes[i].length());
        if (width < length)
          width = length;
      }
    }
    // print prime count results
    std::cout << std::endl;
    for (int i = 0; i < primeSieve.COUNTS_SIZE; i++) {
      if (flags & (COUNT_PRIMES << i))
        std::cout << std::setw(width) << primes[i] << " : " <<
            primeSieve.getCounts(i) << std::endl;
    }
    std::cout << std::setw(width) << "Time elapsed" << " : " <<
        primeSieve.getTimeElapsed() << " sec" << std::endl;
  }
  catch (std::exception& ex) {
    std::cerr << ex.what() << std::endl;
    std::exit(EXIT_FAILURE);
  }
  return 0;
}
