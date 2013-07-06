////////////////////////////////////////////////////////////////////
// parallel_callback.cpp
// Sum the primes below n using 4 threads.
// This version does not scale well using many threads because
// prime callback is synchronized.

#include <primesieve/soe/ParallelPrimeSieve.h>
#include <stdint.h>
#include <cstdlib>
#include <iostream>

uint64_t sum = 0;

void callback(uint64_t prime) {
  sum += prime;
}

int main(int, char** argv)
{
  uint64_t n = 1000000000;
  if (argv[1])
    n = atol(argv[1]);

  ParallelPrimeSieve pps;
  pps.setNumThreads(4);
  pps.generatePrimes(0, n, callback);

  std::cout << "Sum of the primes below " << n << " = " << sum << std::endl;
  return 0;
}
