/*
 * test.cpp -- This file is part of primesieve
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
 * @file  test.cpp
 * @brief Contains various test routines to check if PrimeSieve
 *        produces correct results (single-threaded).
 */

#include "../soe/PrimeSieve.h"
#include "../soe/pmath.h"

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <exception>
#include <ctime>

/**
 * @def __STDC_LIMIT_MACROS
 * Enable the UINT32_MAX macro from <stdint.h>.
 */
#if !defined(__STDC_LIMIT_MACROS)
#  define __STDC_LIMIT_MACROS 1
#endif
#include <stdint.h>

namespace {
  uint64_t primeCounts[] = {
    4,         // pi(101)
    25,        // pi(10^2)
    168,       // pi(10^3)
    1229,      // pi(10^4)
    9592,      // pi(10^5)
    78498,     // pi(10^6)
    664579,    // pi(10^7)
    5761455,   // pi(10^8)
    50847534,  // pi(10^9)
    203280221, // pi(2^32)
    455052511, // pi(10^10)
    155428406, // prime count 2^32 interval starting at 10^12
    143482916, // prime count 2^32 interval starting at 10^13
    133235063, // prime count 2^32 interval starting at 10^14
    124350420, // prime count 2^32 interval starting at 10^15
    116578809, // prime count 2^32 interval starting at 10^16
    109726486, // prime count 2^32 interval starting at 10^17
    103626726, // prime count 2^32 interval starting at 10^18
    98169972}; // prime count 2^32 interval starting at 10^19

  /// Set to true if one or more tests failed
  bool isError = false;
}

/**
 * Evaluate a sieving test, sets isError to true if one or more tests
 * fail.
 */
void evaluateTest(bool isSuccess) {
  if (isSuccess)
    std::cout << "OK" << std::endl;
  else {
    std::cerr << "ERROR" << std::endl;
    isError  = true;
  }
}

/**
 * Sieve about 1000 small random intervals starting at 10^14 until an
 * overall interval of 2^32 has been completed and compare the
 * accumulated prime count result with the correct value from the
 * lookup table.
 */
void testRandomIntervals() {
  std::cout << "Sieve random intervals starting at 10^14" << std::endl;

  uint64_t lowerBound = ipow(10, 14);
  uint64_t upperBound = lowerBound + ipow(2, 32);
  uint64_t maxInterval = ipow(10, 7);
  uint64_t primeCount = 0;

  try {
    std::srand(static_cast<unsigned int> (std::time(0)));
    PrimeSieve primeSieve;
    primeSieve.setStartNumber(lowerBound - 1);
    primeSieve.setStopNumber(lowerBound - 1);
    primeSieve.setFlags(COUNT_PRIMES);

    while (primeSieve.getStopNumber() < upperBound) {
      // generate a rondom 64 bit integer
      uint64_t rand64 = 2 + std::rand();
      while (rand64 < UINT32_MAX)
        rand64 *= rand64;

      // generate a random interval >= 0 and < 10^7
      uint64_t interval = rand64 % maxInterval;
      // generate a random sieve size >= 1 and <= 128
      uint32_t sieveSize = 1 << static_cast<uint32_t> (rand64 % 8);

      // set up primeSieve for the next random interval
      primeSieve.setStartNumber(primeSieve.getStopNumber() + 1);
      primeSieve.setStopNumber(primeSieve.getStartNumber() + interval);
      primeSieve.setSieveSize(sieveSize);
      if (primeSieve.getStopNumber() > upperBound)
        primeSieve.setStopNumber(upperBound);

      // start sieving primes
      primeSieve.sieve();
      // sum the prime count results
      primeCount += primeSieve.getPrimeCount();
      std::cout << "\rRemaining chunk:           "
                << "\rRemaining chunk: " << upperBound - primeSieve.getStopNumber()
                << std::flush;
    }
    std::cout << std::endl;
    std::cout << "Prime count: " << std::setw(11) << primeCount;
    evaluateTest(primeCount == primeCounts[13]);
  }
  catch (std::exception& ex) {
    std::cerr << "Exception " << ex.what() << std::endl;
    std::exit(EXIT_FAILURE);
  }
}

/**
 * Calculate the prime-counting function pi(x) for some popular values
 * of x and compare the results with the correct values from the
 * lookup table.
 */
void testPix() {
  std::cout << "Calculate the prime-counting function pi(x)" << std::endl;
  try {
    PrimeSieve primeSieve;
    primeSieve.setStartNumber(0);
    primeSieve.setStopNumber(0);
    primeSieve.setSieveSize(32);
    primeSieve.setFlags(COUNT_PRIMES);

    uint64_t primeCount = 0;

    // calculate pi(x) for 10^x with x := 1 to 9
    for (int i = 0; i < 9; i++) {
      primeSieve.setStartNumber(primeSieve.getStopNumber() + 1);
      primeSieve.setStopNumber(ipow(10, i + 1));
      primeSieve.sieve();
      primeCount += primeSieve.getPrimeCount();
      std::cout << "pi(10^" << i + 1 << ")  = " << std::setw(12) << primeCount;
      evaluateTest(primeCount == primeCounts[i]);
    }

    // calculate pi(2^32)
    primeSieve.setStartNumber(primeSieve.getStopNumber() + 1);
    primeSieve.setStopNumber(ipow(2, 32));
    primeSieve.sieve();
    primeCount += primeSieve.getPrimeCount();
    std::cout << "pi(2^32)  = " << std::setw(12) << primeCount;
    evaluateTest(primeCount == primeCounts[9]);

    // calculate pi(10^10)
    primeSieve.setStartNumber(primeSieve.getStopNumber() + 1);
    primeSieve.setStopNumber(ipow(10, 10));
    primeSieve.sieve();
    primeCount += primeSieve.getPrimeCount();
    std::cout << "pi(10^10) = " << std::setw(12) << primeCount;
    evaluateTest(primeCount == primeCounts[10]);
  }
  catch (std::exception& ex) {
    std::cerr << "Exception " << ex.what() << std::endl;
    std::exit(EXIT_FAILURE);
  }
}

/**
 * Count the prime numbers within the 2^32 interval starting at 10^x
 * with x := 12 to 19 and compare the results with the correct values
 * from the lookup table.
 */
void testBigPrimes() {
  try {
    int flags = COUNT_PRIMES | PRINT_STATUS;
    PrimeSieve primeSieve;
    primeSieve.setSieveSize(512);
    primeSieve.setFlags(flags);

    for (int i = 0; i <= 7; i++) {
      primeSieve.setStartNumber(ipow(10, 12 + i));
      primeSieve.setStopNumber(primeSieve.getStartNumber() + ipow(2, 32));
      std::cout << "Sieve an interval of 2^32 starting at 10^" << 12 + i << std::endl;
      primeSieve.sieve();
      std::cout << "\rPrime count: " << std::setw(11) << primeSieve.getPrimeCount();
      evaluateTest(primeSieve.getPrimeCount() == primeCounts[11 + i]);
    }
  }
  catch (std::exception& ex) {
    std::cerr << "Exception " << ex.what() << std::endl;
    std::exit(EXIT_FAILURE);
  }
}

/**
 * Run various sieving tests to check if PrimeSieve produces correct
 * results.
 *
 * The test may fail for one of the following reasons:
 *
 * 1. The source code has been modified and a new bug has been
 *    introduced somewhere.
 * 2. The compiler has produced an erroneous executable.
 * 3. The user's system is not stable, i.e. overclocked PC.
 */
void test() {
  std::cout.setf(std::ios::left);

  std::clock_t begin = std::clock();
  // perform random sieving test
  testRandomIntervals();
  std::cout << std::endl;
  // calculate some popular values of pi(x)
  testPix();
  std::cout << std::endl;
  // allocates up to 1 GB of memory
  testBigPrimes();
  std::clock_t stop = std::clock();
  std::cout << std::endl;

  std::cout << "Time elapsed: "
            << ((stop - begin) / static_cast<double> (CLOCKS_PER_SEC))
            << " sec" << std::endl;
  std::cout << ((!isError ) ?"All tests passed SUCCESSFULLY!"
                            :"One or more tests FAILED!")
            << std::endl;
}
