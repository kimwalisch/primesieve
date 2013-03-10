////////////////////////////////////////////////////////////////////
// nth_prime.cpp
// Find the nth prime.
// Usage: $ ./nth_prime 999

#include <primesieve/soe/PrimeSieve.h>
#include <iostream>
#include <cstdlib>
#include <limits>
#include <exception>

class stop_primesieve : public std::exception { };

int i = 0;
int n = 100000000;

void nthprime(unsigned int prime)
{
  if (++i == n) {
    std::cout << n << "th prime = " << prime << std::endl;
    throw stop_primesieve();
  }
}

int main(int, char** argv)
{
  if (argv[1])
    n = atoi(argv[1]);
  try {
    PrimeSieve ps;
    ps.generatePrimes(0, std::numeric_limits<int>::max(), nthprime);
  }
  catch (stop_primesieve&) { }
  return 0;
}
