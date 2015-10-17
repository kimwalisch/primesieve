/** @example previous_prime.c
 *  Iterate backwards over primes using primesieve_iterator. */

#include <primesieve.h>
#include <inttypes.h>
#include <stdio.h>

int main()
{
  primesieve_iterator pi;
  primesieve_init(&pi);

  /* primesieve_skipto(primesieve_iterator, start_number, stop_hint) */
  primesieve_skipto(&pi, 2000, 1000);
  uint64_t prime;

  /* iterate backwards over the primes between 2000 and 1000 */
  while ((prime = primesieve_previous_prime(&pi)) >= 1000)
    printf("%" PRIu64 "\n", prime);

  primesieve_free_iterator(&pi);
  return 0;
}
