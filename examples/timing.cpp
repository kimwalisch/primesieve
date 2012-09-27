////////////////////////////////////////////////////////////////////
// timing.cpp
// getSeconds() returns the time elapsed in seconds.

#include <primesieve/soe/PrimeSieve.h>
#include <stdint.h>
#include <iostream>

int main()
{
  PrimeSieve ps;
  uint64_t count = ps.getPrimeCount(2, 1000000000);
  double seconds = ps.getSeconds();
  std::cout << "Primes below 10^9: " << count << std::endl
            << "Time elapsed: " << seconds << " sec" << std::endl;
  return 0;
}
