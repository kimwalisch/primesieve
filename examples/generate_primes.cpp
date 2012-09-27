////////////////////////////////////////////////////////////////////
// generate_primes.cpp
// Generate the primes up to 1000 and print them.

#include <primesieve/soe/PrimeSieve.h>
#include <iostream>

void callback(unsigned int prime) {
  std::cout << prime << '\n';
}

int main()
{
  PrimeSieve ps;
  ps.generatePrimes(2, 1000, callback);
  return 0;
}
