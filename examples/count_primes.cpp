////////////////////////////////////////////////////////////////////
// count_primes.cpp
// Count the primes up to 1000.

#include <primesieve/soe/PrimeSieve.h>
#include <iostream>

int main()
{
  PrimeSieve ps;
  ps.countPrimes(2, 1000);
  std::cout << "Primes below 1000 = " << ps.getPrimeCount() << std::endl;
  return 0;
}
