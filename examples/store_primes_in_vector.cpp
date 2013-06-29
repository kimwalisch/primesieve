////////////////////////////////////////////////////////////////////
// store_primes_in_vector.cpp
// Store primes in a vector using PrimeSieve.

#include <primesieve/soe/PrimeSieve.h>
#include <vector>

int main()
{
  std::vector<int> primes;
  PrimeSieve ps;

  // Store primes between 100 and 200
  int start = 100, stop = 200;
  ps.generatePrimes(start, stop, &primes);

  primes.clear();

  // Store first 100 primes
  int n = 100;
  ps.generate_N_Primes(n, &primes);

  return 0;
}
