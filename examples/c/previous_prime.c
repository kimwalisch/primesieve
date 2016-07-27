/** @example previous_prime.c
 *  Iterate backwards over primes using primesieve_iterator. */

#include <primesieve.h>
#include <inttypes.h>
#include <stdio.h>

int main()
{
  primesieve_iterator it;
  primesieve_init(&it);

  /* primesieve_skipto(primesieve_iterator, start_number, stop_hint) */
  primesieve_skipto(&it, 2000, 1000);
  uint64_t prime;

  /* iterate backwards over the primes between 2000 and 1000 */
  while ((prime = primesieve_previous_prime(&it)) >= 1000)
    printf("%" PRIu64 "\n", prime);

  primesieve_free_iterator(&it);
  return 0;
}
