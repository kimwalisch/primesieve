/**
 * @example prev_prime.c
 * Iterate backwards over primes using primesieve_iterator.
 * 
 * Note that primesieve_next_prime() runs up to 2x faster and uses
 * only half as much memory as primesieve_prev_prime(). Hence if
 * it is possible to write the same algorithm using either
 * primesieve_prev_prime() or primesieve_next_prime() then it is
 * preferable to use primesieve_next_prime().
 */

#include <primesieve.h>
#include <inttypes.h>
#include <stdio.h>

int main()
{
  primesieve_iterator it;
  primesieve_init(&it);

  /* primesieve_skipto(&it, start_number, stop_hint) */
  primesieve_skipto(&it, 2000, 1000);
  uint64_t prime;

  /* iterate over primes from 2000 to 1000 */
  while ((prime = primesieve_prev_prime(&it)) >= 1000)
    printf("%" PRIu64 "\n", prime);

  primesieve_free_iterator(&it);

  return 0;
}
