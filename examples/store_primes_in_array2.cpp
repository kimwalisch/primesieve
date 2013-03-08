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
  void callback(T prime) { buffer.push_back(prime); }
  std::vector<T> buffer;
};

int main()
{
  PrimeSieveVector<uint64_t> primes;
  PrimeSieve ps;
  ps.generatePrimes(0, 1000, &primes);
  std::cout << primes.buffer.size() << " primes stored!" << std::endl;
  return 0;
}
