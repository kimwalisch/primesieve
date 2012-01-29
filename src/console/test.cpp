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

/** 
 * @file test.cpp
 * @brief `make test` or `./primesieve -test` runs sieving tests to
 *        check if primesieve produces correct results.
 */

#include "../soe/ParallelPrimeSieve.h"

#include <stdint.h>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <algorithm>

namespace {

unsigned int primeCounts[21] = {
  4,           // pi(10^1)
  25,          // pi(10^2)
  168,         // pi(10^3)
  1229,        // pi(10^4)
  9592,        // pi(10^5)
  78498,       // pi(10^6)
  664579,      // pi(10^7)
  5761455,     // pi(10^8)
  50847534,    // pi(10^9)
  203280221,   // pi(2^32)
  455052511,   // pi(10^10)
  4118054813U, // pi(10^11)
  155428406,   // pi[10^12, 10^12+2^32]
  143482916,   // pi[10^13, 10^13+2^32]
  133235063,   // pi[10^14, 10^14+2^32]
  124350420,   // pi[10^15, 10^15+2^32]
  116578809,   // pi[10^16, 10^16+2^32]
  109726486,   // pi[10^17, 10^17+2^32]
  103626726,   // pi[10^18, 10^18+2^32]
  98169972,    // pi[10^19, 10^19+2^32]
  2895317534U  // pi[10^15, 10^15+10^11]
};

// Keeps the memory requirement below 1GB in testBigPrimes()
int maxThreads[8] = { 32, 32, 32, 32, 32, 8, 4, 1 };
// Set to true if any test fails
bool isError = false;
// Time elapsed in seconds of all sieving tests
double seconds = 0.0;

/**
 * Integer pow, raise to power, x^n.
 * Code from (ported to C++ from Ruby):
 * http://en.wikipedia.org/wiki/Exponentiation_by_squaring
 */
uint64_t ipow(uint64_t x, int n) {
  uint64_t result = 1;
  while (n != 0) {
    if (n & 1) {
      result *= x;
      n -= 1;
    }
    x *= x;
    n /= 2;
  }
  return result;
}

void evaluateTest(bool isSuccess) {
  if (isSuccess)
    std::cout << "OK" << std::endl;
  else {
    std::cout << "ERROR" << std::endl;
    isError  = true;
  }
}

/**
 * Calculate the prime-counting function pi(x) up to 10^11
 * and check the primeCounts[] results.
 */
void testPix() {
  std::cout << "Calculating the prime-counting function pi(x)" << std::endl;
  try {
    ParallelPrimeSieve pps;
    pps.setStart(0);
    pps.setStop(0);
    uint64_t primeCount = 0;

    // pi(x) for 10^x with x = 1 to 9
    for (int i = 1; i <= 9; i++) {
      primeCount += pps.getPrimeCount(pps.getStop() + 1, ipow(10, i));
      seconds += pps.getTimeElapsed();
      std::cout << "pi(10^" << i << ")  = " << std::setw(12) << primeCount;
      evaluateTest(primeCount == primeCounts[i-1]);
    }
    // pi(2^32)
    primeCount += pps.getPrimeCount(pps.getStop() + 1, ipow(2, 32));
    seconds += pps.getTimeElapsed();
    std::cout << "pi(2^32)  = " << std::setw(12) << primeCount;
    evaluateTest(primeCount == primeCounts[9]);

    pps.setFlags(pps.COUNT_PRIMES | pps.PRINT_STATUS);
    // pi(x) for 10^x with x = 10 to 11
    for (int i = 10; i <= 11; i++) {
      pps.setStart(pps.getStop() + 1);
      pps.setStop(ipow(10, i));
      pps.sieve();
      primeCount += pps.getPrimeCount();
      seconds += pps.getTimeElapsed();
      std::cout << "\rpi(10^" << i << ") = " << std::setw(12) << primeCount;
      evaluateTest(primeCount == primeCounts[i]);
    }
  }
  catch (std::exception& e) {
    std::cerr << "Exception " << e.what() << std::endl;
    std::exit(EXIT_FAILURE);
  }
}

/**
 * Count the primes within the interval [10^x, 10^x+2^32] with x = 12
 * to 19 and check the primeCounts[] results.
 * @remark Uses up to 1GB of memory
 */
void testBigPrimes() {
  try {
    ParallelPrimeSieve pps;
    pps.setFlags(pps.COUNT_PRIMES | pps.PRINT_STATUS);
    pps.setSieveSize(512);

    for (int i = 0; i < 8; i++) {
      std::cout << "Sieving the primes within [10^" << i+12 << ", 10^" << i+12 << "+2^32]" << std::endl;
      pps.setStart(ipow(10, i + 12));
      pps.setStop(pps.getStart() + ipow(2, 32));
      if (pps.getNumThreads() > maxThreads[i])
        pps.setNumThreads(maxThreads[i]);
      pps.sieve();
      seconds += pps.getTimeElapsed();
      std::cout << "\rPrime count: " << std::setw(11) << pps.getPrimeCount();
      evaluateTest(pps.getPrimeCount() == primeCounts[i + 12]);
    }
  }
  catch (std::exception& e) {
    std::cerr << "Exception " << e.what() << std::endl;
    std::exit(EXIT_FAILURE);
  }
}

/** Generate a random 64-bit integer >= 0 && < limit */
uint64_t getRand64(uint64_t limit) {
  uint64_t rand64 = 0;
  for (int i = 0; i < 4; i++)
    rand64 = std::rand() % (1 << 16) + (rand64 << (16 * i));
  return rand64 % limit;
}

/** Generate a random (power of 2) sieve size >= 1 and <= 4096 */
uint32_t getRandomSieveSize() {
  return static_cast<uint32_t>(1 << (std::rand() % 13));
}

/**
 * Sieve about 200 small random intervals (using random sieve sizes)
 * until the interval [10^15, 10^15+10^11] has been completed and
 * check the prime count result.
 */
void testRandomIntervals() {
  std::cout << "Sieving the primes within [10^15, 10^15+10^11] randomly" << std::endl;
  std::srand(static_cast<unsigned int>(std::time(0)));
  uint64_t lowerBound = ipow(10, 15);
  uint64_t upperBound = lowerBound + ipow(10, 11);
  uint64_t primeCount = 0;
  const uint64_t maxInterval = ipow(10, 9);
  try {
    ParallelPrimeSieve pps;
    pps.setStart(lowerBound - 1);
    pps.setStop(lowerBound - 1);
    pps.setFlags(pps.COUNT_PRIMES);

    while (pps.getStop() < upperBound) {
      pps.setStart(pps.getStop() + 1);
      pps.setStop(pps.getStart() + getRand64(maxInterval));
      if (pps.getStop() > upperBound)
        pps.setStop(upperBound);
      pps.setSieveSize(getRandomSieveSize());
      pps.sieve();
      primeCount += pps.getPrimeCount();
      seconds += pps.getTimeElapsed();
      std::cout << "\rRemaining chunk:             "
                << "\rRemaining chunk: " << upperBound - pps.getStop()
                << std::flush;
    }
    std::cout << std::endl;
    std::cout << "Prime count: " << std::setw(11) << primeCount;
    evaluateTest(primeCount == primeCounts[20]);
  }
  catch (std::exception& e) {
    std::cerr << "Exception " << e.what() << std::endl;
    std::exit(EXIT_FAILURE);
  }
}

} // annonymous namespace

/**
 * Run various sieving tests to check if PrimeSieve produces correct
 * results, uses up to 1GB of memory.
 *
 * The test may fail for one of the following reasons:
 * 1. The source code has been modified and a new bug has been
 *    introduced somewhere.
 * 2. The compiler has produced an erroneous executable.
 * 3. The user's system is not stable.
 */
void test() {
  std::cout << std::left;
  testPix();
  std::cout << std::endl;
  testBigPrimes();
  std::cout << std::endl;
  testRandomIntervals();
  std::cout << std::endl
            << "Time elapsed: " << seconds << " sec" << std::endl
            << ((!isError ) ? "All tests passed SUCCESSFULLY!" : "One or more tests FAILED!")
            << std::endl;
}
