/// @example previous_prime.cpp
/// This example shows how to iterate backwards over primes.

#include <primesieve.h>
#include <iostream>

int main()
{
  primesieve::iterator pi;
  pi.skip_to(2000);

  // backwards print the primes between 1000 and 2000  
  while (pi.previous_prime() > 1000)
    std::cout << pi.prime() << std::endl;

  return 0;
}
