////////////////////////////////////////////////////////////////////
// cancel_prime_generation.cpp
// Cancel prime number generation (callback) by throwing
// a stop_primesieve() exception.

#include <primesieve/soe/PrimeSieve.h>
#include <iostream>
#include <exception>
#include <vector>

class stop_primesieve : public std::exception { };

std::vector<int> primes;

void store(unsigned int prime)
{
  if (primes.size() == 1000) throw stop_primesieve();
  primes.push_back(prime);
}

int main()
{
  PrimeSieve ps;
  try {
    ps.generatePrimes(0, 999999999, store);
  }
  catch (stop_primesieve&) { }
  std::cout << primes.size() << " primes stored!" << std::endl;
  return 0;
}
