/// @example count_primes2.c
/// This example shows how to count primes.

#include <primesieve.h>
#include <stdint.h>
#include <stdio.h>

int main()
{
  uint64_t count = count_primes(0, 1000);
  printf("Primes below 1000 = %llu\n", count);

  // use multi-threading for large intervals
  uint64_t stop = 1ull << 32;
  count = parallel_count_primes(0, stop, MAX_THREADS);
  printf("Primes below 2^32 = %llu\n", count);

  return 0;
}
