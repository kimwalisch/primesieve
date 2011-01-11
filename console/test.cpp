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
 * @file test.cpp
 * @brief Contains various test routines to check if PrimeSieve
 * produces correct results.
 */

#include "test.h"
#include "../src/PrimeSieve.h"
#include "../src/pmath.h"

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <exception>
#include <ctime>

int64_t primeCounts[] = {
  4,         // pi(1e1)
  25,        // pi(1e2)
  168,       // pi(1e3)
  1229,      // pi(1e4)
  9592,      // pi(1e5)
  78498,     // pi(1e6)
  664579,    // pi(1e7)
  5761455,   // pi(1e8)
  50847534,  // pi(1e9)
  203280221, // pi(2^32)
  455052511, // pi(1e10)
  155428406, // prime count of the 2^32 interval starting at 1e12
  143482916, // prime count of the 2^32 interval starting at 1e13
  133235063, // prime count of the 2^32 interval starting at 1e14
  124350420, // prime count of the 2^32 interval starting at 1e15
  116578809, // prime count of the 2^32 interval starting at 1e16
  109726486, // prime count of the 2^32 interval starting at 1e17
  103626726, // prime count of the 2^32 interval starting at 1e18
  98169972}; // prime count of the 2^32 interval starting at 1e19

/// Is set to true if one or more tests fail
bool error = false;

/**
 * Evaluates a sieving test, sets error to true if one or more tests
 * fail.
 */
void evaluateTest(bool passed) {
  if (passed == true)
    std::cout << "OK" << std::endl;
  else {
    std::cout << "ERROR" << std::endl;
    error = true;
  }
}

/**
 * Sieves about 1000 small random intervals starting at 1e14 until an
 * overall interval of 2^32 has been completed. Compares the
 * accumulated prime count result with the correct value from the
 * lookup table.
 */
void testRandomIntervals() {
  std::cout << "Sieve random intervals starting at 1e14" << std::endl;
  uint64_t lowerBound = ipow(10, 14);
  uint64_t upperBound = lowerBound + ipow(2, 32);
  uint64_t maxInterval = ipow(10, 7);
  int64_t primeCount = 0;
  try {
    srand(static_cast<unsigned int> (time(0)));
    PrimeSieve primeSieve;
    primeSieve.setStartNumber(lowerBound - 1);
    primeSieve.setStopNumber(lowerBound - 1);
    primeSieve.setFlags(COUNT_PRIMES);
    do {
      // generate a rondom 64 bit integer
      uint64_t rand64 = 1;
      while (rand64 < UINT32_MAX)
        rand64 *= rand();
      // generate a random interval >= 0 && < 1e7
      uint64_t interval = rand64 % maxInterval;
      // generate a random sieve size >= 1 && <= 128
      uint32_t sieveSize = 1 << (rand64 % 8);
      // set up primeSieve for the next random interval
      primeSieve.setStartNumber(primeSieve.getStopNumber() + 1);
      primeSieve.setStopNumber(primeSieve.getStartNumber() + interval);
      primeSieve.setSieveSize(sieveSize);
      if (primeSieve.getStopNumber() > upperBound)
        primeSieve.setStopNumber(upperBound);
      // start sieving primes
      primeSieve.sieve();
      // accumulate prime count results
      primeCount += primeSieve.getPrimeCount();
      std::cout << "\rRemaining chunk: " << upperBound - primeSieve.getStopNumber()
          << "          " << std::flush;
    } while (primeSieve.getStopNumber() < upperBound);
    // finished
    std::cout << std::endl << "Prime count: " << std::setw(11) << primeCount;
    evaluateTest(primeCount == primeCounts[13]);
  } catch (std::exception& ex) {
    std::cerr << "Exception " << ex.what() << std::endl;
    exit(EXIT_FAILURE);
  }
}

/**
 * Calculates the prime-counting function pi(x) for
 * x = 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 2^32 and 1e10
 * and compares the results with the correct values from the lookup
 * table.
 */
void testPix() {
  std::cout << "Calculate the prime-counting function pi(x)" << std::endl;
  try {
    PrimeSieve primeSieve;
    primeSieve.setStartNumber(0);
    primeSieve.setStopNumber(0);
    primeSieve.setSieveSize(32);
    primeSieve.setFlags(COUNT_PRIMES);
    int64_t primeCount = 0;
    for (int i = 0; i < 9; i++) {
      primeSieve.setStartNumber(primeSieve.getStopNumber() + 1);
      primeSieve.setStopNumber(ipow(10, i + 1));
      primeSieve.sieve();
      primeCount += primeSieve.getPrimeCount();
      std::cout << "pi(1e" << i + 1 << ")  = " << std::setw(13) << primeCount;
      evaluateTest(primeCount == primeCounts[i]);
    }
    primeSieve.setStartNumber(primeSieve.getStopNumber() + 1);
    primeSieve.setStopNumber(ipow(2, 32));
    primeSieve.sieve();
    primeCount += primeSieve.getPrimeCount();
    std::cout << "pi(2^32) = " << std::setw(13) << primeCount;
    evaluateTest(primeCount == primeCounts[9]);
    primeSieve.setStartNumber(primeSieve.getStopNumber() + 1);
    primeSieve.setStopNumber(ipow(10, 10));
    primeSieve.sieve();
    primeCount += primeSieve.getPrimeCount();
    std::cout << "pi(1e10) = " << std::setw(13) << primeCount;
    evaluateTest(primeCount == primeCounts[10]);
  }
  catch (std::exception& ex) {
    std::cerr << "Exception " << ex.what() << std::endl;
    exit(EXIT_FAILURE);
  }
}

/**
 * Counts the prime numbers within the 2^32 interval starting at 1e12,
 * 1e13, 1e14, 1e15, 1e16, 1e17, 1e18 and 1e19 and compares the
 * results with the correct values from the lookup table.
 */
void testBigPrimes() {
  try {
    PrimeSieve primeSieve;
    primeSieve.setSieveSize(512);
    primeSieve.setFlags(COUNT_PRIMES | PRINT_STATUS);
    for (int i = 0; i <= 7; i++) {
      primeSieve.setStartNumber(ipow(10, 12 + i));
      primeSieve.setStopNumber(primeSieve.getStartNumber() + ipow(2, 32));
      std::cout << "Sieve an interval of 2^32 starting at 1e" << 12 + i << std::endl;
      primeSieve.sieve();
      std::cout << "Prime count: " << std::setw(11) << primeSieve.getPrimeCount();
      evaluateTest(primeSieve.getPrimeCount() == primeCounts[11 + i]);
    }
  }
  catch (std::exception& ex) {
    std::cerr << "Exception " << ex.what() << std::endl;
    exit(EXIT_FAILURE);
  }
}

/**
 * Test PrimeSieve.
 *
 * Runs various sieving tests to see if PrimeSieve produces correct
 * results. The test may fail for the following reasons:
 * 1. The source code has been modified and a new bug has been
 *    introduced somewhere.
 * 2. The compiler has produced an erroneous executable.
 * 3. The user's system is not stable, i.e. overclocked PC.
 */
void test() {
  std::cout.setf(std::ios::left);
  std::clock_t begin = std::clock();
  testRandomIntervals();
  std::cout << std::endl;
  testPix();
  std::cout << std::endl;
  testBigPrimes();
  std::cout << std::endl;
  std::cout << "Time elapsed: " << ((std::clock() - begin) / 
      static_cast<double> (CLOCKS_PER_SEC)) << " sec" << std::endl;
  if (!error)
    std::cout << "All tests passed SUCCESSFULLY!" << std::endl;
  else
    std::cout << "One or more tests FAILED!" << std::endl;
}
