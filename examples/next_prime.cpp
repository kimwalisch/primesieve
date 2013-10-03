/// @example next_prime.cpp
/// This example shows how to iterate over primes.

#include <primesieve.h>
#include <iostream>

int main()
{
  primesieve::prime_iterator pi;
  pi.skip_to(1000);

  // print the primes between 1000 and 2000
  while (pi.next_prime() < 2000)
    std::cout << pi.prime() << std::endl;

  return 0;
}
