/// @example primesieve_iterator.cpp
/// Iterate over primes using primesieve::iterator.

#include <primesieve.hpp>
#include <chrono>
#include <cstdlib>
#include <iostream>

int main(int argc, char** argv)
{
  uint64_t limit = 10000000000;

  if (argc > 1)
    limit = std::atoll(argv[1]);

  auto t1 = std::chrono::steady_clock::now();

  primesieve::iterator it(0, limit);
  uint64_t prime = it.next_prime();
  uint64_t sum = 0;

  // Iterate over the primes <= 10^9
  for (; prime <= limit; prime = it.next_prime())
    sum += prime;

  auto t2 = std::chrono::steady_clock::now();
  std::chrono::duration<double> seconds = t2 - t1;

  std::cout << "Sum of primes <= " << limit << ": " << sum << std::endl;
  std::cout << "Seconds: " << seconds.count() << std::endl;

  return 0;
}
