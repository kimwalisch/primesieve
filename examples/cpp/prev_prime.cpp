/// @example prev_prime.cpp
/// Iterate backwards over primes using primesieve::iterator.

#include <primesieve.hpp>
#include <iostream>

int main()
{
  primesieve::iterator it;
  it.skipto(2000);
  uint64_t prime = it.prev_prime();

  // iterate over primes from 2000 to 1000
  for (; prime >= 1000;  prime = it.prev_prime())
    std::cout << prime << std::endl;

  return 0;
}
