/*
 * main.cpp -- This file is part of primesieve
 *
 * Copyright (C) 2010 Kim Walisch, <kim.walisch@gmail.com>
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
 * @file main.cpp
 * @brief Command line version of PrimeSieve.
 * 
 * PrimeSieve is a highly optimized implementation of the sieve of
 * Eratosthenes that finds prime numbers and prime k-tuplets
 * (twin primes, prime triplets, ...) up to 2^64.
 */

#include "PrimeSieve.h"
#include "pmath.h"
#include "utils/strtoull.h"

#include <stdint.h>
#include <exception>
#include <iostream>
#include <cstdlib>
#include <cctype>
#include <ctime>

// PrimeSieve arguments
uint64_t start = 0;      /* lower bound for sieving */
uint64_t stop = 0;       /* upper bound for sieving */
uint32_t flags = 0;      /* settings */
uint32_t sieveSize = 64; /* sieve size in KiloBytes */

std::string primes[7] = { "Prime numbers", "Twin primes", "Prime triplets",
    "Prime quadruplets", "Prime quintuplets", "Prime sextuplets",
    "Prime septuplets" };

void version() {
  std::cout << "primesieve 1.02, <http://primesieve.googlecode.com>"
      << std::endl << "Copyright (C) 2010 Kim Walisch" << std::endl
      << "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>."
      << std::endl
      << "This is free software: you are free to change and redistribute it."
      << std::endl << "There is NO WARRANTY, to the extent permitted by law."
      << std::endl;
  exit(EXIT_SUCCESS);
}

void help() {
  std::cout << "Usage: primesieve START STOP [OPTION]" << std::endl
      << "Use the sieve of Eratosthenes to find the prime numbers and prime"
      << std::endl << "k-tuplets between START and STOP < 2^64" << std::endl
      << "Example: primesieve 1 1000 -p1" << std::endl << std::endl
      << "Options:" << std::endl
      << "  -s <size>  Set the sieve size (in KiloBytes)," << std::endl
      << "             size >= 1 && size <= 8192" << std::endl
      << "             Set size to your CPU's L1 or L2 cache size for best performance"
      << std::endl;
  for (char c = '1'; c <= '7'; c++)
    std::cout << "  -c" << c << "        Count " << primes[c - '1']
        << std::endl;
  for (char c = '1'; c <= '7'; c++)
    std::cout << "  -p" << c << "        Print " << primes[c - '1']
        << std::endl;
  std::cout << "  -v         Print version and license information and exit"
      << std::endl;
  exit(EXIT_SUCCESS);
}

/** Process command line arguments. */
void processOptions(int argc, char* argv[]) {
  if (argc == 1 || argc > 2 * 7 + 3)
    help();
  int i = 1;
  if (argc > 2) {
    start = utils::strtoull(argv[i++]);
    stop  = utils::strtoull(argv[i++]);
  }
  for (; i < argc; i++) {
    if (*argv[i] == '-' || *argv[i] == '/')
      argv[i]++;
    int x = -1;
    switch (std::tolower(*argv[i])) {
    case 'c': /* set count flags */
      x = argv[i][1] - '0';
      if (x < 1 || x > 7)
        help();
      flags |= COUNT_PRIMES << (x - 1);
      break;
    case 'p': /* set print flag */
      x = argv[i][1] - '0';
      if (x < 1 || x > 7)
        help();
      flags |= PRINT_PRIMES << (x - 1);
      break;
    case 's': /* set sieve size */
      if (argv[++i] == NULL)
        help();
      sieveSize = std::strtoul(argv[i], NULL, 10);
      if (sieveSize < 1 || sieveSize > 8192)
        help();
      break;
    case 'v': /* print version information */
      version();
      break;
    default:
      help();
    }
  }
}

int main(int argc, char* argv[]) {
  processOptions(argc, argv);
  if (start > stop) {
    std::cerr << "START must be <= STOP" << std::endl;
    exit(EXIT_FAILURE);
  }
  if (stop >= UINT64_MAX - UINT32_MAX * UINT64_C(10)) {
    std::cerr << "STOP must be < (2^64-1) - (2^32-1) * 10." << std::endl;
    exit(EXIT_FAILURE);
  }
  // count prime numbers if none else selected
  if ((flags & COUNT_FLAGS) == 0)
    flags |= COUNT_PRIMES;
  // make sure sieve size is a power of 2
  sieveSize = nextHighestPowerOf2(sieveSize);
  if ((flags & PRINT_FLAGS) == 0) {
    // print the status whilst sieving
    flags |= PRINT_STATUS;
    std::cout << "Sieve size set to " << sieveSize << " KiloBytes" << std::endl;
  }
  try {
    std::clock_t begin = std::clock();
    // Initialize primeSieve
    PrimeSieve primeSieve;
    primeSieve.setStartNumber(start);
    primeSieve.setStopNumber(stop);
    primeSieve.setSieveSize(sieveSize);
    primeSieve.setFlags(flags);
    // start sieving primes (using the sieve of Eratosthenes)
    primeSieve.sieve();
    std::clock_t end = std::clock();
    // print prime count results
    for (int i = 0; i < 7; i++) {
      if (primeSieve.getCounts(i) >= 0)
        std::cout << primes[i] << ": "
            << primeSieve.getCounts(i) << std::endl;
    }
    std::cout << "Time elapsed: " << ((end - begin)
        / static_cast<double> (CLOCKS_PER_SEC)) << " sec" << std::endl;
  } catch (std::exception& ex) {
    std::cerr << argv[0] << " - " << ex.what();
    exit(EXIT_FAILURE);
  }
  return 0;
}
