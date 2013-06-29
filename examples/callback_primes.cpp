////////////////////////////////////////////////////////////////////
// callback_primes.cpp
// The callback function will be executed for each prime
// in the interval [2, 1000].

#include <primesieve/soe/PrimeSieve.h>
#include <iostream>

void callback(unsigned int prime)
{
  std::cout << prime << ", ";
}

int main()
{
  PrimeSieve ps;
  ps.generatePrimes(2, 1000, callback);
  return 0;
}
