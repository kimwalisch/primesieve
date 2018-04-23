/// @example store_primes_in_vector.cpp
/// Store primes in a std::vector using primesieve.

#include <primesieve.hpp>
#include <vector>

int main()
{
  std::vector<int> primes;

  // Store primes <= 1000
  primesieve::generate_primes(1000, &primes);

  primes.clear();

  // Store primes inside [1000, 2000]
  primesieve::generate_primes(1000, 2000, &primes);

  primes.clear();

  // Store first 1000 primes
  primesieve::generate_n_primes(1000, &primes);

  primes.clear();

  // Store first 10 primes >= 1000
  primesieve::generate_n_primes(10, 1000, &primes);

  return 0;
}
