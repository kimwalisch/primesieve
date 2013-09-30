////////////////////////////////////////////////////////////////////
// Backwards print the primes between 1000 and 2000.

#include <primesieve.h>
#include <iostream>

int main()
{
  primesieve::prime_iterator pi;
  pi.skip_to(2000);

  uint64_t prime;
  while ((prime = pi.previous_prime()) > 1000)
    std::cout << prime << std::endl;

  return 0;
}
