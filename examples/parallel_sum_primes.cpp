////////////////////////////////////////////////////////////////////
// parallel_sum_primes.cpp
// Sum the primes below 10^10 using 4 threads.

#include <primesieve/soe/ParallelPrimeSieve.h>
#include <stdint.h>
#include <iostream>

uint64_t sum = 0;

void callback(uint64_t prime) {
  sum += prime;
}

int main()
{
  ParallelPrimeSieve pps;
  pps.setNumThreads(4);
  pps.generatePrimes(0, (uint64_t) 1E10, callback);
  std::cout << "Sum of the primes below 10^10 = " << sum << std::endl;
  return 0;
}
