////////////////////////////////////////////////////////////////////
// parallel_sum_primes2.cpp
// Sum the primes below 10^11 using all CPU cores.

#include <primesieve/soe/ParallelPrimeSieve.h>
#include <stdint.h>
#include <iostream>
#include <vector>
#include <numeric>

const int THREADS = ParallelPrimeSieve::getMaxThreads();
const int CACHE_LINE = 256;
const int NO_FALSE_SHARING = CACHE_LINE / sizeof(uint64_t);

std::vector<uint64_t> sum;

void callback(uint64_t prime, int threadNumber)
{
  int threadIndex = threadNumber * NO_FALSE_SHARING;
  sum[threadIndex] += prime;
}

int main()
{
  sum.resize(THREADS * NO_FALSE_SHARING, 0);
  ParallelPrimeSieve pps;
  pps.setNumThreads(THREADS);
  pps.generatePrimes(0, (uint64_t) 1E11, callback);
  std::cout << "Sum of the primes below 10^11 = "
            << std::accumulate(sum.begin(), sum.end(), 0ull)
            << std::endl;
  return 0;
}
