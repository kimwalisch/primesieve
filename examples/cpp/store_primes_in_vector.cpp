/// @example store_primes_in_vector.cpp
/// Store primes in a std::vector using primesieve.

#include <primesieve.hpp>
#include <vector>

int main()
{
  std::vector<int> primes;

  // Store the primes below 1000
  primes.clear();
  primesieve::generate_primes(1000, &primes);

  // Store the primes within the interval [1000, 2000]
  primes.clear();
  primesieve::generate_primes(1000, 2000, &primes);

  // Store first 1000 primes
  primes.clear();
  primesieve::generate_n_primes(1000, &primes);

  return 0;
}
