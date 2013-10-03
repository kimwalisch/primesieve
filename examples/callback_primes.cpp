/// @example callback_primes.cpp
/// This example shows how to use callback functions.

#include <primesieve.h>
#include <iostream>

void callback(unsigned int prime)
{
  std::cout << prime << ", ";
}

int main()
{
  primesieve::callback_primes(2, 1000, callback);
  return 0;
}
