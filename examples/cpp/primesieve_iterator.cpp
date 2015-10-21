/// @example primesieve_iterator.cpp
/// Iterate over primes using a primesieve::iterator object.

#include <primesieve.hpp>
#include <iostream>

int main()
{
  primesieve::iterator pi;
  uint64_t sum = 0;
  uint64_t prime;

  // iterate over primes below 10^10
  while ((prime = pi.next_prime()) < 10000000000ull)
    sum += prime;

  std::cout << "Sum of the primes below 10^10 = " << sum << std::endl;
  return 0;
}
