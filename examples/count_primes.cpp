////////////////////////////////////////////////////////////////////
// count_primes.cpp
// Count the primes up to 1000.

#include <primesieve.h>
#include <stdint.h>
#include <iostream>

int main()
{
  PrimeSieve ps;
  uint64_t count = primesieve::count_primes(0, 1000);
  std::cout << "Primes below 1000 = " << count << std::endl;
  return 0;
}
