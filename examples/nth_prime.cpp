////////////////////////////////////////////////////////////////////
// nth_prime.cpp
// Find the nth prime.
// Usage: $ ./nth_prime 999

#include <primesieve/soe/ParallelPrimeSieve.h>
#include <iostream>
#include <cstdlib>

int main(int, char** argv)
{
  long n = 100000000;
  if (argv[1])
    n = atol(argv[1]);

  ParallelPrimeSieve pps;
  std::cout << n << "th prime = " << pps.nthPrime(n) << std::endl;
  return 0;
}
