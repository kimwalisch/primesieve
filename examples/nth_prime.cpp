////////////////////////////////////////////////////////////////////
// nth_prime.cpp
// Usage: $ ./nth_prime 999

#include <primesieve/soe/ParallelPrimeSieve.h>
#include <stdint.h>
#include <iostream>
#include <cstdlib>

int main(int, char** argv)
{
  uint64_t n = 1000;
  if (argv[1])
    n = atol(argv[1]);

  ParallelPrimeSieve pps;
  uint64_t nthPrime = pps.nthPrime(n);
  std::cout << n << "th prime = " << nthPrime << std::endl;
  return 0;
}
