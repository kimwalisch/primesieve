////////////////////////////////////////////////////////////////////
// cancel_prime_generation.cpp
// Store the first 1000 primes in a vector.

#include <primesieve/soe/PrimeSieve.h>
#include <iostream>
#include <exception>
#include <vector>

std::vector<unsigned int> primes;

void store(unsigned int prime)
{
  // cancel by throwing an exception
  if (primes.size() == 1000) throw std::exception();
  primes.push_back(prime);
}

int main()
{
  PrimeSieve ps;
  try {
    ps.generatePrimes(0, 999999999, store);
  }
  catch (std::exception&) { }
  std::cout << "1000th prime = " << primes.back() << std::endl;
  return 0;
}
