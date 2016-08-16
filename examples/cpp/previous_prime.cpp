/// @example previous_prime.cpp
/// Iterate backwards over primes using primesieve::iterator.

#include <primesieve.hpp>
#include <iostream>

int main()
{
  primesieve::iterator it;
  it.skipto(2000);
  uint64_t prime = it.previous_prime();

  // iterate over the primes from 2000 to 1000
  for (; prime >= 1000;  prime = it.previous_prime())
    std::cout << prime << std::endl;

  return 0;
}
