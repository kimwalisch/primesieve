/// @example nth_prime.cpp
/// Find the nth prime.

#include <primesieve.hpp>
#include <stdint.h>
#include <iostream>
#include <cstdlib>

int main(int, char** argv)
{
  uint64_t n = 1000;

  if (argv[1])
    n = std::atol(argv[1]);

  uint64_t nth_prime = primesieve::nth_prime(n);
  std::cout << n << "th prime = " << nth_prime << std::endl;

  return 0;
}
