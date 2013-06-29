////////////////////////////////////////////////////////////////////
// callback_primes_oop.cpp
// Instances of classes that derive from PrimeSieveCallback<T>
// can be passed to the generatePrimes() methods.

#include <primesieve/soe/PrimeSieve.h>
#include <primesieve/soe/PrimeSieveCallback.h>
#include <stdint.h>
#include <list>

struct PrimeList : std::list<uint64_t>,
                   PrimeSieveCallback<uint64_t> {
  void callback(uint64_t prime)
  {
    this->push_back(prime);
  }
};

int main()
{
  PrimeList primeList;
  PrimeSieve ps;
  ps.generatePrimes(0, 1000, &primeList);
  return 0;
}
