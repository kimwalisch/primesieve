/// @example primesieve_iterator.cpp
/// Iterate over primes using primesieve::iterator.

#include <primesieve.hpp>
#include <iostream>

int main()
{
  primesieve::iterator it;
  uint64_t prime = it.next_prime();
  uint64_t sum = 0;

  // iterate over the primes below 10^10
  for (; prime < 10000000000ull; prime = it.next_prime())
    sum += prime;

  std::cout << "Sum of the primes below 10^10 = " << sum << std::endl;
  return 0;
}
