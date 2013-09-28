////////////////////////////////////////////////////////////////////
// parallel_callback_faster.cpp
// Sum the primes below n using multiple threads.
// This version scales well using many threads because
// prime callback is not synchronized.

#include <primesieve.h>
#include <stdint.h>
#include <cstdlib>
#include <iostream>
#include <vector>

const int CACHE_LINE = 256;
const int NO_FALSE_SHARING = CACHE_LINE / sizeof(uint64_t);

std::vector<uint64_t> sums;

/// unsynchronized callback function
void callback(uint64_t prime, int threadNum)
{
  int threadIndex = threadNum * NO_FALSE_SHARING;
  sums[threadIndex] += prime;
}

int main(int, char** argv)
{
  uint64_t n = 1000000000;
  if (argv[1])
    n = atol(argv[1]);

  sums.resize(threads * NO_FALSE_SHARING, 0);
  primesieve::parallel_callback_primes(0, n, callback);

  uint64_t sum = 0;
  for (uint64_t i = 0; i < sums.size(); i += NO_FALSE_SHARING)
    sum += sums[i];

  std::cout << "Sum of the primes below " << n << " = " << sum << std::endl;
  return 0;
}
