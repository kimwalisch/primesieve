////////////////////////////////////////////////////////////////////
// timing.cpp
// getSeconds() returns the time elapsed in seconds.

#include <primesieve/soe/PrimeSieve.h>
#include <iostream>

int main()
{
  PrimeSieve ps;
  ps.countPrimes(2, 1000000000);
  std::cout << "Time elapsed: " << ps.getSeconds() << " sec" << std::endl;
  return 0;
}
