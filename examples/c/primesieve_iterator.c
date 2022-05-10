/** @example primesieve_iterator.c
 *  Iterate over primes using C primesieve_iterator. */

#include <primesieve.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
  uint64_t limit = 10000000000ull;

  if (argc > 1)
    limit = atol(argv[1]);

  primesieve_iterator it;
  primesieve_init(&it);

  /* indicate exact bounds to improve performance */
  primesieve_skipto(&it, 0, limit);

  uint64_t sum = 0;
  uint64_t prime = 0;

  /* iterate over the primes below 10^9 */
  while ((prime = primesieve_next_prime(&it)) <= limit)
    sum += prime;

  printf("Sum of the primes <= %" PRIu64 " = %" PRIu64 "\n", limit, sum);

  primesieve_free_iterator(&it);

  return 0;
}
