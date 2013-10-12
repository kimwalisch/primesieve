/// @example cancel_callback.cpp
/// Cancel callback of primes by throwing a stop_primesieve()
/// exception (no need to catch it).
/// Note that multi-threaded callback can currently not be canceled.

#include <primesieve.h>
#include <stdint.h>
#include <iostream>

int i = 0;

void callback(uint64_t prime)
{
  if (++i == 1000000) {
    std::cout << "10^6th prime = " << prime << std::endl;
    throw stop_primesieve();
  }
}

int main()
{
  primesieve::callback_primes(0, 999999999, callback);
  return 0;
}
