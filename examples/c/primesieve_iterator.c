/** @example primesieve_iterator.c
 *  Iterate over primes using C primesieve_iterator. */

#include <primesieve.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char** argv)
{
  uint64_t limit = 10000000000;

  if (argc > 1)
    limit = atoll(argv[1]);

  clock_t t1 = clock();

  primesieve_iterator it;
  primesieve_init(&it);

  /* Indicate exact bounds to improve performance */
  primesieve_jump_to(&it, 0, limit);

  uint64_t sum = 0;
  uint64_t prime = 0;

  /* Iterate over the primes <= 10^9 */
  while ((prime = primesieve_next_prime(&it)) <= limit)
    sum += prime;

  clock_t t2 = clock();
  double seconds = (double) (t2 - t1) / CLOCKS_PER_SEC;

  printf("Sum of the primes <= %" PRIu64 ": %" PRIu64 "\n", limit, sum);
  printf("Seconds: %g\n", seconds);

  primesieve_free_iterator(&it);

  return 0;
}
