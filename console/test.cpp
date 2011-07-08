//
// Copyright (c) 2011 Kim Walisch, <kim.walisch@gmail.com>.
// All rights reserved.
//
// This file is part of primesieve.
// Visit: http://primesieve.googlecode.com
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
//   * Neither the name of the modp.com nor the names of its
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

/** 
 * @file  test.cpp
 * @brief Contains various test routines to check if PrimeSieve
 *        produces correct results.
 */

#include "../soe/ParallelPrimeSieve.h"
#include "../soe/imath.h"

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <algorithm>

/**
 * @def __STDC_LIMIT_MACROS
 * Enable the UINT32_MAX macro from <stdint.h>.
 */
#if !defined(__STDC_LIMIT_MACROS)
#  define __STDC_LIMIT_MACROS 1
#endif
#include <stdint.h>

namespace {
  uint32_t primeCounts[19] = {
      4,         // pi(10^1)
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
      155428406, // prime count of 2^32 interval at 10^12
      143482916, // prime count of 2^32 interval at 10^13
      133235063, // prime count of 2^32 interval at 10^14
      124350420, // prime count of 2^32 interval at 10^15
      116578809, // prime count of 2^32 interval at 10^16
      109726486, // prime count of 2^32 interval at 10^17
      103626726, // prime count of 2^32 interval at 10^18
      98169972}; // prime count of 2^32 interval at 10^19
    
  /// Keeps the memory requirement below 1 GB in testBigPrimes()
  int maxThreads[8] = {32, 32, 32, 32, 32, 8, 4, 1};
  /// Set to true if one or more tests failed
  bool isError = false;
  /// Time elapsed in seconds of all sieving tests
  double seconds = 0.0;

  void evaluateTest(bool isSuccess) {
    if (isSuccess)
      std::cout << "OK" << std::endl;
    else {
      std::cout << "ERROR" << std::endl;
      isError  = true;
    }
  }

  /**
   * Sieve about 1000 small random intervals starting at 10^14 until
   * an overall interval of 2^32 has been completed and compare the
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
      ParallelPrimeSieve pps;
      pps.setStartNumber(lowerBound - 1);
      pps.setStopNumber(lowerBound - 1);
      pps.setFlags(ParallelPrimeSieve::COUNT_PRIMES);

      while (pps.getStopNumber() < upperBound) {
        // generate a rondom 64 bit integer
        uint64_t rand64 = 1;
        while (rand64 < UINT32_MAX)
          rand64 += rand64 * std::max<int> (2, std::rand());
        // generate a random interval >= 0 and < 10^7
        uint64_t interval = rand64 % maxInterval;
        // generate a random sieve size >= 1 and <= 128
        uint32_t sieveSize = 1 << static_cast<int> (rand64 % 8);

        // set up pps for the next random interval
        pps.setStartNumber(pps.getStopNumber() + 1);
        pps.setStopNumber(pps.getStartNumber() + interval);
        pps.setSieveSize(sieveSize);
        if (pps.getStopNumber() > upperBound)
          pps.setStopNumber(upperBound);
        // start sieving primes
        pps.sieve();
        primeCount += pps.getPrimeCount();
        seconds += pps.getTimeElapsed();
        std::cout << "\rRemaining chunk:           "
                  << "\rRemaining chunk: " << upperBound - pps.getStopNumber()
                  << std::flush;
      }
      std::cout << std::endl;
      std::cout << "Prime count: " << std::setw(11) << primeCount;
      evaluateTest(primeCount == primeCounts[13]);
    }
    catch (std::exception& e) {
      std::cerr << "Exception " << e.what() << std::endl;
      std::exit(EXIT_FAILURE);
    }
  }

  /**
   * Calculate the prime-counting function pi(x) for some popular
   * values of x and compare the results with the correct values from
   * the lookup table.
   */
  void testPix() {
    std::cout << "Calculate the prime-counting function pi(x)" << std::endl;
    try {
      ParallelPrimeSieve pps;
      pps.setStartNumber(0);
      pps.setStopNumber(0);
      pps.setSieveSize(32);
      pps.setFlags(pps.COUNT_PRIMES);
      int i = 0;
      uint64_t primeCount = 0;

      // calculate pi(x) for 10^x with x := 1 to 9
      for (; i < 9; i++) {
        primeCount += pps.getPrimeCount(pps.getStopNumber() + 1, ipow(10, i + 1));
        seconds += pps.getTimeElapsed();
        std::cout << "pi(10^" << i + 1 << ")  = " << std::setw(12) << primeCount;
        evaluateTest(primeCount == primeCounts[i]);
      }
      // calculate pi(2^32)
      primeCount += pps.getPrimeCount(pps.getStopNumber() + 1, ipow(2, 32));
      seconds += pps.getTimeElapsed();
      std::cout << "pi(2^32)  = " << std::setw(12) << primeCount;
      evaluateTest(primeCount == primeCounts[i++]);
      // calculate pi(10^10)
      primeCount += pps.getPrimeCount(pps.getStopNumber() + 1, ipow(10, 10));
      seconds += pps.getTimeElapsed();
      std::cout << "pi(10^10) = " << std::setw(12) << primeCount;
      evaluateTest(primeCount == primeCounts[i++]);
    }
    catch (std::exception& e) {
      std::cerr << "Exception " << e.what() << std::endl;
      std::exit(EXIT_FAILURE);
    }
  }

  /**
   * Count the prime numbers within a 2^32 interval starting at 10^x
   * with x := 12 to 19 and compare the results with the correct
   * values from the lookup table.
   * @remark uses up to 1GB of memory
   */
  void testBigPrimes() {
    try {
      ParallelPrimeSieve pps;
      pps.setSieveSize(512);
      pps.setFlags(pps.COUNT_PRIMES | pps.PRINT_STATUS);

      for (int i = 0; i < 8; i++) {
        pps.setStartNumber(ipow(10, 12 + i));
        pps.setStopNumber(pps.getStartNumber() + ipow(2, 32));
        std::cout << "Sieve an interval of 2^32 starting at 10^" << 12 + i << std::endl;
        if (pps.getNumThreads() > maxThreads[i])
          pps.setNumThreads(maxThreads[i]);
        pps.sieve();
        seconds += pps.getTimeElapsed();
        std::cout << "\rPrime count: " << std::setw(11) << pps.getPrimeCount();
        evaluateTest(pps.getPrimeCount() == primeCounts[11 + i]);
      }
    }
    catch (std::exception& e) {
      std::cerr << "Exception " << e.what() << std::endl;
      std::exit(EXIT_FAILURE);
    }
  }
} // end anonymous namespace

/**
 * Run various sieving tests to check if PrimeSieve produces correct
 * results.
 * @remark uses up to 1GB of memory
 *
 * The test may fail for one of the following reasons:
 *
 * 1. The source code has been modified and a new bug has been
 *    introduced somewhere.
 * 2. The compiler has produced an erroneous executable.
 * 3. The user's system is not stable.
 */
void test() {
  std::cout.setf(std::ios::left);
  // run tests
  testRandomIntervals();
  std::cout << std::endl;
  testPix();
  std::cout << std::endl;
  testBigPrimes();
  std::cout << std::endl;
  std::cout << "Time elapsed: " << seconds << " sec" << std::endl
            << ((!isError ) ? "All tests passed SUCCESSFULLY!" : "One or more tests FAILED!")
            << std::endl;
}
