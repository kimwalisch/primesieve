#include <primesieve.hpp>
#include <iostream>
#include <omp.h>

int main()
{
  uint64_t sum = 0;
  uint64_t dist = 1e10;
  int threads = omp_get_max_threads();
  uint64_t thread_dist = (dist / threads) + 1;

  #pragma omp parallel for reduction(+: sum)
  for (int i = 0; i < threads; i++)
  {
    uint64_t start = i * thread_dist;
    uint64_t stop = std::min(start + thread_dist, dist);
    primesieve::iterator it(start, stop);
    uint64_t prime = it.next_prime();

    for (; prime <= stop; prime = it.next_prime())
      sum += prime;
  }

  std::cout << "Sum of the primes below " << dist << ": " << sum << std::endl;

  return 0;
}
