//
// Copyright (c) 2012 Kim Walisch, <kim.walisch@gmail.com>.
// All rights reserved.
//
// This file is part of primesieve.
// Homepage: http://primesieve.googlecode.com
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//   * Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above
//     copyright notice, this list of conditions and the following
//     disclaimer in the documentation and/or other materials provided
//     with the distribution.
//   * Neither the name of the author nor the names of its
//     contributors may be used to endorse or promote products derived
//     from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.


/// @file  main.cpp
/// @brief Command-line version of primesieve, multi-threaded (OpenMP).
/// @see   http://primesieve.googlecode.com
/// primesieve is a highly optimized implementation of the sieve of
/// Eratosthenes that generates prime numbers and prime k-tuplets (twin
/// primes, prime triplets, ...) up to 2^64 maximum.

#include "../expr/ExpressionParser.h"
#include "../soe/ParallelPrimeSieve.h"

// declared in test.cpp
void test();

#include <stdint.h>
#include <exception>
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>

namespace {

// start and stop number for sieving
std::vector<uint64_t> numbers;

int32_t threads    = -1;
int32_t flags      =  0;
int32_t sieveSize  = -1;
int32_t preSieve   = -1;

bool quietMode   = false;
bool printParser = false;

const std::string primes[7] = {
  "Prime numbers",
  "Twin primes",
  "Prime triplets",
  "Prime quadruplets",
  "Prime quintuplets", 
  "Prime sextuplets",
  "Prime septuplets"
};

enum {
  OPTION_ERROR,
  OPTION_HELP,
  OPTION_TEST,
  OPTION_VERSION,
  START_SIEVING
};

void help() {
  const int maxThreads = ParallelPrimeSieve::getMaxThreads();
  std::cout << "Usage: primesieve START STOP [OPTION]..."                                              << std::endl
            << "Use the segmented sieve of Eratosthenes to generate the prime numbers and/or"          << std::endl
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
            << "                the sieve of Eratosthenes, 13 <= PRE-SIEVE <= 23"                      << std::endl
            << "  -s<SIZE>      Set the sieve size in kilobytes, 1 <= SIZE <= 4096"                    << std::endl
            << "                Set SIZE to your CPU's L1/L2 cache size for best performance"          << std::endl
            << "  -t<THREADS>   Set the number of threads for sieving, 1 <= THREADS <= " << maxThreads << std::endl
            << "                Primes are not generated in order if THREADS >= 2"                     << std::endl
            << "  -test         Run various sieving tests and exit"                                    << std::endl
            << "  -v            Print version and license information and exit"                        << std::endl
                                                                                                       << std::endl
            << "Examples:"                                                                             << std::endl
                                                                                                       << std::endl
            << "Print the prime numbers up to 1000:"                                                   << std::endl
            << "> primesieve 2 1000 -p1"                                                               << std::endl
                                                                                                       << std::endl
            << "Count the twin primes and prime triplets in the interval [1E10, 1E10+2^32]:"           << std::endl
            << "> primesieve 1E10 -o2**32 -c23"                                                        << std::endl;
}

void version() {
  std::cout << "primesieve 3.7, <http://primesieve.googlecode.com>" << std::endl
            << "Copyright (C) 2012 Kim Walisch" << std::endl
            << "This software is licensed under the New BSD License. See the LICENSE file" << std::endl
            << "for more information." << std::endl;
}

bool isDigits(const std::string &str) {
  const std::string digits("0123456789");
  if (str.size() == 0)
    return false;
  return str.find_first_not_of(digits) == std::string::npos;
}

/// Process the command-line options.
/// @see help(void)
///
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
        printParser = true;
    }
  }
  // process options
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
                preSieve = static_cast<int32_t>(parser.getResult());
                break;
      case 's': parser.eval(argv[i]);
                if (!parser.isSuccess())
                  return OPTION_HELP;
                sieveSize = static_cast<int32_t>(parser.getResult());
                break;
      case 't': arg = argv[i];
                if (arg.compare(testOption) == 0)
                  return OPTION_TEST;
                parser.eval(argv[i]);
                if (!parser.isSuccess())
                  return OPTION_HELP;
                threads = static_cast<int32_t>(parser.getResult());
                break;
      case 'v': return OPTION_VERSION;
      default : return OPTION_HELP;
    }
  }
  return (numbers.size() == 2) ? START_SIEVING : OPTION_HELP;
}

} // end namespace

int main(int argc, char* argv[]) {
  // process the command-line options, see help()
  switch(processOptions(argc, argv)) {
    case OPTION_ERROR:              return 1;
    case OPTION_HELP:    help();    return 0;
    case OPTION_VERSION: version(); return 0;
    case OPTION_TEST:    test();    return 0;
    case START_SIEVING:  break;
  }
  std::cout << std::left;
  if (!quietMode && printParser) {
    std::cout << std::setw(10) << "START" << " = " << numbers[0] << std::endl;
    std::cout << std::setw(10) << "STOP"  << " = " << numbers[1] << std::endl;
  }
  try {
    ParallelPrimeSieve pps;
    pps.setStart(numbers[0]);
    pps.setStop(numbers[1]);
    if (flags     !=  0) pps.setFlags(flags);
    if (sieveSize != -1) pps.setSieveSize(sieveSize);
    if (preSieve  != -1) pps.setPreSieveLimit(preSieve);
    if (threads   != -1) pps.setNumThreads(threads);

    // set default settings
    if (!pps.isPrint() && !pps.isCount())
      pps.addFlags(pps.COUNT_PRIMES);
    if (!pps.isPrint() && !quietMode)
      pps.addFlags(pps.PRINT_STATUS);

    if (!quietMode) {
      if (preSieve != -1)
      std::cout << std::setw(10) << "Pre-sieve"  << " = " << pps.getPreSieveLimit() << std::endl;
      std::cout << std::setw(10) << "Sieve size" << " = " << pps.getSieveSize() << " kilobytes" << std::endl;
      std::cout << std::setw(10) << "Threads"    << " = " << pps.getNumThreads() << std::endl;
    }

    // start sieving primes
    pps.sieve();

    if (pps.isFlag(pps.PRINT_STATUS))
      std::cout << std::endl;
    if (pps.isPrint() && pps.isCount())
      std::cout << std::endl;

    // get max string size
    std::size_t size = 0;
    for (int i = 0; i < 7; i++) {
      if (pps.isFlag(pps.COUNT_PRIMES << i))
        size = std::max(size, primes[i].size());
    }
    int width = static_cast<int>(size);
    // print prime count results
    for (int32_t i = 0; i < 7; i++) {
      if (pps.isFlag(pps.COUNT_PRIMES << i))
        std::cout << std::setw(width)
                  << primes[i] << " : " << pps.getCounts(i)
                  << std::endl;
    }
    if (!pps.isPrint()) {
      std::cout << std::setw(width)
                << "Time elapsed" << " : " << pps.getSeconds() << " sec"
                << std::endl;
    }
    if (!quietMode && pps.getNumThreads() >= 64) {
      std::cout << "\nHint: the -q (Quiet mode) option significantly reduces the thread" << std::endl
                << "synchronization overhead when using >= 64 threads." << std::endl;
    }
  }
  catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl
              << "Try `primesieve -help' for more information." << std::endl;
    return 1;
  }
  return 0;
}
