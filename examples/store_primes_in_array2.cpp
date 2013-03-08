////////////////////////////////////////////////////////////////////
// store_primes_in_array2.cpp
// Store the primes below 1000 in a PrimeSieveVector.

#include <primesieve/soe/PrimeSieve.h>
#include <primesieve/soe/PrimeSieveCallback.h>
#include <stdint.h>
#include <vector>
#include <iostream>

template <typename T>
struct PrimeSieveVector : public PrimeSieveCallback<T>
{
  void callback(T prime) { primes.push_back(prime); }
  std::vector<T> primes;
};

int main()
{
  PrimeSieveVector<uint64_t> vect;
  PrimeSieve ps;
  ps.generatePrimes(0, 1000, &vect);
  std::cout << vect.primes.size() << " primes stored!" << std::endl;
  return 0;
}
