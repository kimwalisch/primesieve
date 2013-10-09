/// @example cancel_callback.cpp
/// Cancel prime callback by throwing a stop_primesieve()
/// exception. This code finds the 10^6th prime.
/// Note that multi-threaded callback can currently not be canceled.

#include <primesieve.h>
#include <iostream>

int i = 0;

void callback(unsigned int prime)
{
  if (++i == 1000000) {
    std::cout << "10^6th prime = " << prime << std::endl;
    throw stop_primesieve();
  }
}

int main()
{
  try {
    primesieve::callback_primes(0, 999999999, callback);
  }
  catch (stop_primesieve&) { }
  return 0;
}
