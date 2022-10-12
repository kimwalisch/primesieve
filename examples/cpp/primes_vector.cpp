/// @example primes_vector.cpp
/// Fill a std::vector with primes.

#include <primesieve.hpp>
#include <vector>

int main()
{
  std::vector<int> primes;

  // Fill the primes vector with the primes <= 1000
  primesieve::generate_primes(1000, &primes);

  primes.clear();

  // Fill the primes vector with the primes inside [1000, 2000]
  primesieve::generate_primes(1000, 2000, &primes);

  primes.clear();

  // Fill the primes vector with the first 1000 primes
  primesieve::generate_n_primes(1000, &primes);

  primes.clear();

  // Fill the primes vector with the first 10 primes >= 1000
  primesieve::generate_n_primes(10, 1000, &primes);

  return 0;
}
