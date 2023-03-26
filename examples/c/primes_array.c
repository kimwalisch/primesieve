/** @example primes_array.c
 *  Generate an array of primes. */

#include <primesieve.h>
#include <stdio.h>

int main(void)
{
  uint64_t start = 0;
  uint64_t stop = 1000;
  size_t i;
  size_t size;

  /* Get an array with the primes <= 1000 */
  int* primes = (int*) primesieve_generate_primes(start, stop, &size, INT_PRIMES);

  for (i = 0; i < size; i++)
    printf("%i\n", primes[i]);

  primesieve_free(primes);
  uint64_t n = 1000;

  /* Get an array with the first 1000 primes */
  primes = (int*) primesieve_generate_n_primes(n, start, INT_PRIMES);

  for (i = 0; i < n; i++)
    printf("%i\n", primes[i]);

  primesieve_free(primes);
  return 0;
}
