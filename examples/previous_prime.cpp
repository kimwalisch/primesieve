/// @example previous_prime.cpp
/// This example shows how to iterate backwards over primes.

#include <primesieve.hpp>
#include <iostream>

int main()
{
  primesieve::iterator pi;
  pi.skipto(2000);

  uint64_t prime;

  // iterate backwards over the primes between 2000 and 1000
  while ((prime = pi.previous_prime()) >= 1000)
    std::cout << prime << std::endl;

  return 0;
}
