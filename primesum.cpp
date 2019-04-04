#include <primesieve.hpp>
#include <iostream>
#include <omp.h>

int main()
{
  uint64_t prime_sum = 0;
  uint64_t dist = 1ull << 32;
  uint64_t threads = omp_get_max_threads();
  uint64_t thread_dist = dist / threads;

  #pragma omp parallel for reduction(+: prime_sum)
  for (uint64_t i = 0; i < dist; i += thread_dist)
  {
    primesieve::iterator it;
    it.skipto(i);
    uint64_t prime = it.next_prime();
    uint64_t stop = i + thread_dist;

    for (; prime <= stop; prime = it.next_prime())
      prime_sum += prime;
  }

  std::cout << "Sum of the primes below " << dist << ": " << prime_sum << std::endl;

  return 0;
}
