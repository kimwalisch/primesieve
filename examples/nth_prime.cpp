////////////////////////////////////////////////////////////////////
// nth_prime.cpp
// Usage: $ ./nth_prime 999

#include <primesieve.h>
#include <stdint.h>
#include <iostream>
#include <cstdlib>

int main(int, char** argv)
{
  uint64_t n = 1000;
  if (argv[1])
    n = atol(argv[1]);

  uint64_t nth_prime = primesieve::nth_prime(n);
  std::cout << n << "th prime = " << nth_prime << std::endl;
  return 0;
}
