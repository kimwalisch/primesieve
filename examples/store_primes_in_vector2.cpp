////////////////////////////////////////////////////////////////////
// store_primes_in_vector2.cpp
// Instances of classes that derive from PrimeSieveCallback<T>
// can be passed to the generatePrimes() methods.

#include <primesieve/soe/PrimeSieve.h>
#include <primesieve/soe/PrimeSieveCallback.h>
#include <stdint.h>
#include <vector>
#include <iostream>

template <typename T>
struct PrimeSieveVector : public std::vector<T>,
                          public PrimeSieveCallback<uint64_t>
{
  void callback(uint64_t prime)
  {
    this->push_back( static_cast<T>(prime) );
  }
};

int main()
{
  PrimeSieveVector<int> primes;
  PrimeSieve ps;
  ps.generatePrimes(0, 1000, &primes);
  for (std::size_t i = 0; i < primes.size(); i++)
    std::cout << primes[i] << ", ";
  return 0;
}
