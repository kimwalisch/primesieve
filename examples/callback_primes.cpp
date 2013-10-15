/// @example callback_primes.cpp
/// This example shows how to use callback functions.

#include <primesieve.hpp>
#include <stdint.h>
#include <iostream>

void callback(uint64_t prime)
{
  std::cout << prime << std::endl;
}

int main()
{
  primesieve::callback_primes(2, 1000, callback);
  return 0;
}
