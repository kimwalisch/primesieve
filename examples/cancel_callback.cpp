////////////////////////////////////////////////////////////////////
// cancel_callback.cpp
// Cancel prime generation by throwing a stop_primesieve()
// exception. Find the 10^7th prime.

#include <primesieve/soe/PrimeSieve.h>
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
  try {
    PrimeSieve ps;
    ps.generatePrimes(0, n * 50, callback);
  }
  catch (stop_primesieve&) { }
  return 0;
}
