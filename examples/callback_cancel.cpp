////////////////////////////////////////////////////////////////////
// cancel_callback.cpp
// Cancel prime callback by throwing a stop_primesieve()
// exception. Note that multi-threaded callback can currently
// not be canceled. This code finds the 10^7th prime.

#include <primesieve.h>
#include <primesieve/soe/stop_primesieve.h>
#include <iostream>

int i = 0;
int n = 10000000;

void callback(unsigned int prime)
{
  if (++i == n) {
    std::cout << n << "th prime = " << prime << std::endl;
    throw stop_primesieve();
  }
}

int main()
{
  try
  {
    primesieve::callback_primes(0, n * 50, callback);
  }
  catch (stop_primesieve&) { }
  return 0;
}
