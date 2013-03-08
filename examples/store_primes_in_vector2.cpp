////////////////////////////////////////////////////////////////////
// store_primes_in_vector2.cpp
// Store the primes below 1000 in a PrimeSieveVector that is
// derived from PrimeSieveCallback and std::vector.

#include <primesieve/soe/PrimeSieve.h>
#include <primesieve/soe/PrimeSieveCallback.h>
#include <stdint.h>
#include <vector>
#include <iostream>

template <typename T>
// make final for C++11 or later
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
    std::cout << primes[i] << std::endl;
  return 0;
}
