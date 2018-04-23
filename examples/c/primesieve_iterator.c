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

  /* iterate over the primes below 10^9 */
  while ((prime = primesieve_next_prime(&it)) < 1000000000ull)
    sum += prime;

  printf("Sum of the primes below 10^9 = %" PRIu64 "\n", sum);

  /* generate primes > 1000 */
  primesieve_skipto(&it, 1000, 1100);

  while ((prime = primesieve_next_prime(&it)) < 1100)
    printf("%" PRIu64 "\n", prime);

  primesieve_free_iterator(&it);

  return 0;
}
