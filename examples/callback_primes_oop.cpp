/// @example callback_primes_oop.cpp
/// Objects derived from PrimeSieveCallback<T> can be
/// passed to the callback_primes() function.

#include <primesieve.h>
#include <stdint.h>
#include <list>

struct PrimeList : std::list<uint64_t>, PrimeSieveCallback<uint64_t>
{
  void callback(uint64_t prime) {
    this->push_back(prime);
  }
};

int main()
{
  PrimeList primeList;
  primesieve::callback_primes(2, 1000, &primeList);
  return 0;
}
