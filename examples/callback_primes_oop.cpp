/// @example callback_primes_oop.cpp
/// Objects derived from PrimeSieveCallback<T> can be
/// passed to the callback_primes() function.

#include <primesieve.h>
#include <list>

struct PrimeList : std::list<int>,
                   PrimeSieveCallback<int>
{
  void callback(int prime) {
    this->push_back(prime);
  }
};

int main()
{
  PrimeList primeList;
  primesieve::callback_primes(2, 1000, &primeList);
  return 0;
}
