/// @example primesieve_iterator.cpp
/// Iterate over primes using primesieve::iterator.

#include <primesieve.hpp>
#include <cstdlib>
#include <iostream>

int main(int argc, char** argv)
{
  uint64_t limit = 10000000000ull;

  if (argc > 1)
    limit = std::atol(argv[1]);

  primesieve::iterator it(0, limit);
  uint64_t prime = it.next_prime();
  uint64_t sum = 0;

  // Iterate over the primes <= 10^9
  for (; prime <= limit; prime = it.next_prime())
    sum += prime;

  std::cout << "Sum of primes <= " << limit << ": " << sum << std::endl;

  // Note that since sum is a 64-bit variable the result
  // will be incorrect (due to integer overflow) if
  // limit > 10^10. However we do allow limits > 10^10
  // since this is useful for benchmarking.
  if (limit > 10000000000ull)
    std::cerr << "Warning: sum is likely incorrect due to 64-bit integer overflow!" << std::endl;

  return 0;
}
