/// @example prev_prime.cpp
/// Iterate backwards over primes using primesieve::iterator.

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

  primesieve::iterator it;
  it.jump_to(limit);
  uint64_t prime = it.prev_prime();
  uint64_t sum = 0;

  // Backwards iterate over the primes <= 10^9
  for (; prime > 0; prime = it.prev_prime())
    sum += prime;

  auto t2 = std::chrono::steady_clock::now();
  std::chrono::duration<double> seconds = t2 - t1;

  std::cout << "Sum of primes <= " << limit << ": " << sum << std::endl;
  std::cout << "Seconds: " << seconds.count() << std::endl;

  return 0;
}
