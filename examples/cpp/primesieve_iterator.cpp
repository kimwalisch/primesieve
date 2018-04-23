/// @example primesieve_iterator.cpp
/// Iterate over primes using primesieve::iterator.

#include <primesieve.hpp>
#include <iostream>

int main()
{
  primesieve::iterator it;
  uint64_t prime = it.next_prime();
  uint64_t sum = 0;

  // iterate over the primes below 10^9
  for (; prime < 1000000000ull; prime = it.next_prime())
    sum += prime;

  std::cout << "Sum of the primes below 10^9 = " << sum << std::endl;

  // generate primes > 1000
  it.skipto(1000);
  prime = it.next_prime();

  for (; prime < 1100; prime = it.next_prime())
    std::cout << prime << std::endl;

  return 0;
}
