/// @example count_primes.cpp
/// This example shows how to count primes.

#include <primesieve.hpp>
#include <stdint.h>
#include <iostream>

int main()
{
  uint64_t count = primesieve::count_primes(0, 1000);
  std::cout << "Primes below 1000 = " << count << std::endl;

  uint64_t stop = 1000000000;

  // use multi-threading for large intervals
  count = primesieve::parallel_count_primes(0, stop);
  std::cout << "Primes below 10^9 = " << count << std::endl;

  return 0;
}
