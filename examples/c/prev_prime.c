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
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int main(int argc, char** argv)
{
  uint64_t limit = 10000000000;

  if (argc > 1)
    limit = atoll(argv[1]);

  clock_t t1 = clock();

  primesieve_iterator it;
  primesieve_init(&it);

  /* primesieve_jump_to(&it, start_number, stop_hint) */
  primesieve_jump_to(&it, limit, 0);
  uint64_t prime;
  uint64_t sum = 0;

  /* Backwards iterate over the primes <= limit */
  while ((prime = primesieve_prev_prime(&it)) > 0)
    sum += prime;

  clock_t t2 = clock();
  double seconds = (double) (t2 - t1) / CLOCKS_PER_SEC;

  primesieve_free_iterator(&it);
  printf("Sum of the primes: %" PRIu64 "\n", sum);
  printf("Seconds: %g\n", seconds);

  return 0;
}
