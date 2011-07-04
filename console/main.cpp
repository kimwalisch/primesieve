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
 * @brief Command-line version of primesieve, multi-threaded (OpenMP).
 * @see   http://primesieve.googlecode.com
 *
 * primesieve is a highly optimized implementation of the sieve of
 * Eratosthenes that generates prime numbers and prime k-tuplets (twin
 * primes, prime triplets, ...) up to 2^64 maximum.
 */

#include "../soe/ParallelPrimeSieve.h"
#include "../soe/defs.h"
#include "../expr/ExpressionParser.h"

/// declared in test.cpp
void test();

#include <stdint.h>
#include <exception>
#include <iostream>
#include <string>
#include <vector>
#include <iomanip> /* std::setw(int) */

namespace {
  enum {
    OPTION_ERROR,
    OPTION_HELP,
    OPTION_TEST,
    OPTION_VERSION,
    START_SIEVING
  };

  std::vector<uint64_t> numbers; /* start and stop number for sieving */
  uint32_t sieveSize     = 0;    /* sieve size in Kilobytes */
  uint32_t preSieve      = defs::PRIMESIEVE_PRESIEVE_LIMIT;
  uint32_t flags         = 0;    /* settings */
  bool quietMode         = false;
  bool showParserResults = false;
  int threads            = ParallelPrimeSieve::USE_IDEAL_NUM_THREADS;
  int maxThreads         = ParallelPrimeSieve::getMaxThreads();

  // Unfortunately there is no easy way to get the CPU L1 and L2 cache
  // size, these values are close for most x86-64 CPUs in 2011
  const uint32_t L1_CACHE_SIZE = 64;
  const uint32_t L2_CACHE_SIZE = 512;
  const uint64_t L2_THRESHOLD  = static_cast<uint64_t> (1E13);
}

void help() {
  std::cout << "Usage: primesieve START STOP [OPTION]..."                                              << std::endl
            << "Use the segmented sieve of Eratosthenes to generate the prime numbers and"             << std::endl
            << "prime k-tuplets in the interval [START, STOP] < 2^64"                                  << std::endl
                                                                                                       << std::endl
            << "Options:"                                                                              << std::endl
                                                                                                       << std::endl
            << "  -c<N+>        Count prime numbers and/or prime k-tuplets, 1 <= N <= 7"               << std::endl
            << "                e.g. -c1 count prime numbers (DEFAULT)"                                << std::endl
            << "                     -c23 count twin primes and prime triplets"                        << std::endl
            << "  -o<OFFSET>    Sieve the interval [START, START+OFFSET]"                              << std::endl
            << "  -p<N>         Print prime numbers or prime k-tuplets, 1 <= N <= 7"                   << std::endl
            << "                e.g. -p1 print prime numbers"                                          << std::endl
            << "                     -p5 print prime quintuplets"                                      << std::endl
            << "  -q            Quiet mode, print less output"                                         << std::endl
            << "  -r<PRE-SIEVE> Pre-sieve multiples of small primes <= PRE-SIEVE to speed up"          << std::endl
            << "                the sieve of Eratosthenes, 11 <= PRE-SIEVE <= 23"                      << std::endl
            << "  -s<SIZE>      Set the sieve size in Kilobytes, 1 <= SIZE <= 8192"                    << std::endl
            << "                Set SIZE to your CPU's L1/L2 cache size for best performance"          << std::endl
            << "  -t<THREADS>   Set the number of threads for sieving, 1 <= THREADS <= " << maxThreads << std::endl
            << "                Primes are not generated in order if THREADS >= 2"                     << std::endl
            << "  -test         Run various sieving tests and exit"                                    << std::endl
            << "  -v            Print version and license information and exit"                        << std::endl
                                                                                                       << std::endl
            << "Examples:"                                                                             << std::endl
                                                                                                       << std::endl
            << "  primesieve 2 1000 -p1    print the prime numbers up to 1000"                         << std::endl
            << "  primesieve 2 1E11 -s32   count the prime numbers up to 10^11 using a"                << std::endl
            << "                           sieve size of 32 KB"                                        << std::endl;
}

void version() {
  std::cout << "primesieve 2.3, <http://primesieve.googlecode.com>" << std::endl
            << "Copyright (C) 2011 Kim Walisch" << std::endl
            << "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>." << std::endl
            << "This is free software: you are free to change and redistribute it." << std::endl
            << "There is NO WARRANTY, to the extent permitted by law." << std::endl;
}

bool isDigits(const std::string &str) {
  const std::string digits("0123456789");
  if (str.size() == 0)
    return false;
  return str.find_first_not_of(digits) == std::string::npos;
}

/**
 * Process the command-line options.
 * @see help(void)
 */
int processOptions(std::size_t argc, char* argv[]) {
  if (argc < 2 || argc > 20)
    return OPTION_HELP;

  std::string testOption("est");
  std::string arg;
  ExpressionParser<uint64_t> parser;
  uint64_t tmp = 0;

  // process the START and STOP numbers
  for (std::size_t i = 1; i < 3 && i < argc; i++) {
    if (parser.eval(argv[i])) {
      numbers.push_back(parser.getResult());
      if (!isDigits(argv[i]))
        showParserResults = true;
    }
  }
  // process the options ([OPTION]...)
  for (std::size_t i = numbers.size() + 1; i < argc; i++) {
    if (*argv[i] != '-' && *argv[i] != '/')
      return OPTION_HELP;
    argv[i]++;
    switch (*argv[i]++) {
      case 'c': parser.eval(argv[i]);
                if (!parser.isSuccess())
                  return OPTION_HELP;
                tmp = parser.getResult();
                do {
                  if (tmp % 10 < 1 || tmp % 10 > 7)
                    return OPTION_HELP;
                  flags |= ParallelPrimeSieve::COUNT_PRIMES << (tmp % 10 - 1);
                  tmp /= 10;
                } while (tmp > 0);
                break;
      case 'o': parser.eval(argv[i]);
                if (!parser.isSuccess() || numbers.size() == 0)
                  return OPTION_HELP;
                tmp = numbers[0] + parser.getResult();
                numbers.push_back(tmp);
                break;
      case 'p': parser.eval(argv[i]);
                if (!parser.isSuccess() || parser.getResult() < 1 || parser.getResult() > 7)
                  return OPTION_HELP;
                tmp = parser.getResult() - 1;
                flags |= ParallelPrimeSieve::PRINT_PRIMES << tmp;
                quietMode = true;
                break;
      case 'q': quietMode = true;
                break;
      case 'r': parser.eval(argv[i]);
                if (!parser.isSuccess())
                  return OPTION_HELP;
                preSieve = static_cast<uint32_t> (parser.getResult());
                break;
      case 's': parser.eval(argv[i]);
                if (!parser.isSuccess())
                  return OPTION_HELP;
                sieveSize = static_cast<uint32_t> (parser.getResult());
                break;
      case 't': arg = argv[i];
                if (arg.compare(testOption) == 0)
                  return OPTION_TEST;
                parser.eval(argv[i]);
                if (!parser.isSuccess())
                  return OPTION_HELP;
                threads = static_cast<int> (parser.getResult());
                break;
      case 'v': return OPTION_VERSION;
      default : return OPTION_HELP;
    }
  }
  return (numbers.size() == 2) ? START_SIEVING : OPTION_HELP;
}

/**
 * Process the command-line options and start sieving.
 */
int main(int argc, char* argv[]) {
  switch(processOptions(argc, argv)) {
    case OPTION_ERROR:              return 1;
    case OPTION_HELP:    help();    return 0;
    case OPTION_VERSION: version(); return 0;
    case OPTION_TEST:    test();    return 0;
    case START_SIEVING:  break;
  }
  // print parser results
  std::cout.setf(std::ios::left);
  if (!quietMode && showParserResults)
    std::cout << std::setw(10) << "START" << " = " << numbers[0] << std::endl
              << std::setw(10) << "STOP"  << " = " << numbers[1] << std::endl;
  try {
    ParallelPrimeSieve pps;

    // set default settings
    if ((flags & pps.COUNT_FLAGS) == 0 && (flags & pps.PRINT_FLAGS) == 0)
      flags |= pps.COUNT_PRIMES;
    if (!quietMode && (flags & pps.PRINT_FLAGS) == 0)
      flags |= pps.PRINT_STATUS;
    if (sieveSize == 0)
      sieveSize = (numbers[1] < L2_THRESHOLD) ? L1_CACHE_SIZE : L2_CACHE_SIZE;

    pps.setStartNumber(numbers[0]);
    pps.setStopNumber(numbers[1]);
    pps.setSieveSize(sieveSize);
    pps.setPreSieveLimit(preSieve);
    pps.setFlags(flags);
    pps.setNumThreads(threads);

    if (!quietMode)
      std::cout << std::setw(10) << "Sieve size" << " = " << pps.getSieveSize() << " Kilobytes" << std::endl
                << std::setw(10) << "Threads" << " = " << pps.getNumThreads() << std::endl;

    // start sieving primes
    pps.sieve();
    if ((flags & pps.PRINT_STATUS) || (
        (flags & pps.PRINT_FLAGS) &&
        (flags & pps.COUNT_FLAGS)))
      std::cout << std::endl;

    const std::string primes[7] = {
        "Prime numbers",
        "Twin primes",
        "Prime triplets",
        "Prime quadruplets",
        "Prime quintuplets", 
        "Prime sextuplets",
        "Prime septuplets"};

    // get max string size
    std::size_t size = (quietMode) ? 0 : 12;
    for (int i = 0; i < 7; i++)
      if ((flags & (pps.COUNT_PRIMES << i)) && size < primes[i].size())
        size = primes[i].size();
    // print prime count results and time elapsed
    int width = static_cast<int> (size);
    for (int i = 0; i < 7; i++)
      if (flags & (pps.COUNT_PRIMES << i))
        std::cout << std::setw(width) << primes[i] << " : " << pps.getCounts(i) << std::endl;
    if (!quietMode)
      std::cout << std::setw(width) << "Time elapsed" << " : " << pps.getTimeElapsed() << " sec" << std::endl;
  }
  catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl
              << "Try `primesieve -help' for more information." << std::endl;
    return 1;
  }
  return 0;
}
