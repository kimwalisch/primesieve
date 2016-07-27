/** @example primesieve_iterator.c
 *  Iterate over primes using C primesieve_iterator. */

#include <primesieve.h>
#include <inttypes.h>
#include <stdio.h>

int main()
{
  primesieve_iterator it;
  primesieve_init(&it);

  uint64_t sum = 0;
  uint64_t prime = 0;

  /* iterate over the primes below 10^10 */
  while ((prime = primesieve_next_prime(&it)) < 10000000000ull)
    sum += prime;

  primesieve_free_iterator(&it);
  printf("Sum of the primes below 10^10 = %" PRIu64 "\n", sum);
  return 0;
}
