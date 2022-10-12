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

int main(int argc, char** argv)
{
  uint64_t limit = 10000000000ull;

  if (argc > 1)
    limit = atol(argv[1]);

  primesieve_iterator it;
  primesieve_init(&it);

  /* primesieve_jump_to(&it, start_number, stop_hint) */
  primesieve_jump_to(&it, limit, 0);
  uint64_t prime;
  uint64_t sum = 0;

  /* Backwards iterate over the primes <= limit */
  while ((prime = primesieve_prev_prime(&it)) > 0)
    sum += prime;

  primesieve_free_iterator(&it);
  printf("Sum of the primes: %" PRIu64 "\n", sum);

  /* Note that since sum is a 64-bit variable the result
   * will be incorrect (due to integer overflow) if
   * limit > 10^10. However we do allow limits > 10^10
   * since this is useful for benchmarking. */
  if (limit > 10000000000ull)
    printf("Warning: sum is likely incorrect due to 64-bit integer overflow!");

  return 0;
}
