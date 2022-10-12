/// @example count_primes.cpp
/// This example shows how to count primes.

#include <primesieve.hpp>
#include <stdint.h>
#include <iostream>

int main()
{
  uint64_t count = primesieve::count_primes(0, 1000);
  std::cout << "Primes <= 1000: " << count << std::endl;

  return 0;
}
