///
/// @file   primesieve_test.cpp
/// @brief  bool primesieve_test(); runs sieving tests to ensure that
///         ParallelPrimeSieve objects return correct results.
///
/// Copyright (C) 2016 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve.hpp>

#include <stdint.h>
#include <iostream>
#include <iomanip>
#include <exception>
#include <stdexcept>
#include <algorithm>
#include <cstdlib>
#include <ctime>

using namespace std;

namespace primesieve {

/// Correct values to compare with test results
const unsigned int primeCounts[19] =
{
  4,           // pi(10^1)
  25,          // pi(10^2)
  168,         // pi(10^3)
  1229,        // pi(10^4)
  9592,        // pi(10^5)
  78498,       // pi(10^6)
  664579,      // pi(10^7)
  5761455,     // pi(10^8)
  50847534,    // pi(10^9)
  455052511,   // pi(10^10)
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

/// Keeps the memory usage below 1GB
const int maxThreads[8] = { 32, 32, 32, 32, 32, 8, 4, 1 };

uint64_t ipow(uint64_t x, int n)
{
  uint64_t result = 1;
  while (n != 0)
  {
    if ((n & 1) != 0)
    {
      result *= x;
      n -= 1;
    }
    x *= x;
    n /= 2;
  }
  return result;
}

/// Get a random 64-bit integer < limit
uint64_t getRand64(uint64_t limit)
{
  uint64_t rand64 = 0;
  for (int i = 0; i < 4; i++)
    rand64 = rand() % (1 << 16) + (rand64 << (i * 16));
  return rand64 % limit;
}

void check(bool isCorrect)
{
  cout << (isCorrect ? "OK" : "ERROR") << endl;
  if (!isCorrect)
    throw runtime_error("test failed!");
}

/// Count the primes up to 10^10
void testPix()
{
  cout << "pi(x) : Prime-counting function test" << endl;
  ParallelPrimeSieve pps;
  pps.setSieveSize(get_sieve_size());
  pps.setNumThreads(get_num_threads());
  pps.setStart(0);
  pps.setStop(0);
  uint64_t primeCount = 0;

  // pi(x) with x = 10^i for i = 1 to 10
  for (int i = 1; i <= 10; i++)
  {
    primeCount += pps.countPrimes(pps.getStop() + 1, ipow(10, i));
    cout << "pi(10^" << i << (i < 10 ? ")  = " : ") = ") << setw(12) << primeCount;
    check(primeCount == primeCounts[i - 1]);
  }
  cout << endl;
}

/// Count the primes within [10^i, 10^i+2^32] for i = 12 to 19
void testBigPrimes()
{
  ParallelPrimeSieve pps;
  pps.setSieveSize(get_sieve_size());
  pps.setNumThreads(get_num_threads());
  pps.setFlags(pps.COUNT_PRIMES | pps.PRINT_STATUS);

  for (int i = 12; i <= 19; i++)
  {
    cout << "Sieving the primes within [10^" << i << ", 10^" << i << "+2^32]" << endl;
    pps.setStart(ipow(10, i));
    pps.setStop(pps.getStart() + ipow(2, 32));
    pps.setNumThreads(min(pps.getNumThreads(), maxThreads[i - 12]));
    pps.sieve();
    cout << "\rPrime count: " << setw(11) << pps.getPrimeCount();
    check(pps.getPrimeCount() == primeCounts[i - 2]);
  }
  cout << endl;
}

/// Sieve about 200 small random intervals until the interval
/// [10^15, 10^15+10^11] has been completed.
///
void testRandomIntervals()
{
  cout << "Sieving the primes within [10^15, 10^15+10^11] randomly" << endl;
  uint64_t maxDistance = ipow(10, 9);
  uint64_t lowerBound = ipow(10, 15);
  uint64_t upperBound = lowerBound + ipow(10, 11);
  uint64_t primeCount = 0;
  srand(static_cast<unsigned int>(time(0)));
  ParallelPrimeSieve pps;
  pps.setNumThreads(get_num_threads());
  pps.setStart(lowerBound - 1);
  pps.setStop(lowerBound - 1);

  while (pps.getStop() < upperBound)
  {
    pps.setStart(pps.getStop() + 1);
    pps.setStop(min(pps.getStart() + getRand64(maxDistance), upperBound));
    pps.setSieveSize(1 << (rand() % 12));
    pps.sieve();
    primeCount += pps.getPrimeCount();
    cout << "\rRemaining chunk:             "
         << "\rRemaining chunk: "
         << upperBound - pps.getStop() << flush;
  }
  cout << endl << "Prime count: " << setw(11) << primeCount;
  check(primeCount == primeCounts[18]);
  cout << endl;
}

/// Run various sieving tests to ensure that ParallelPrimeSieve
/// (and PrimeSieve) objects produce correct results.
/// The tests use up to 1 gigabyte of memory and take about
/// 1 minute to complete on a quad core CPU from 2013.
/// @return true if success else false.
///
bool primesieve_test()
{
  try
  {
    cout << left;
    testPix();
    testBigPrimes();
    testRandomIntervals();
  }
  catch (exception& e)
  {
    cerr << endl << "primesieve error: " << e.what() << endl;
    return false;
  }
  cout << "All tests passed successfully!" << endl;
  return true;
}

} // namespace
