////////////////////////////////////////////////////////////////////
// nth_prime.cpp
// Find the nth prime.
// Usage: $ ./nth_prime 999

#include <primesieve/soe/PrimeSieve.h>
#include <iostream>
#include <cstdlib>
#include <limits>
#include <exception>

int count = 0;
int stop = 1000;

void nthprime(unsigned int prime)
{
  if (++count == stop) {
    std::cout << count << "th prime = " << prime << std::endl;
    throw std::exception();
  }
}

int main(int, char** argv)
{
  if (argv[1])
    stop = atoi(argv[1]);
  try {
    PrimeSieve ps;
    ps.generatePrimes(0, std::numeric_limits<int>::max(), nthprime);
  }
  catch (std::exception&) { }
  return 0;
}
