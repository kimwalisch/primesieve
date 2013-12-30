/** @example count_primes.c
 *  C program that shows how to count primes. */

#include <primesieve.h>
#include <stdint.h>
#include <stdio.h>

int main()
{
  uint64_t count = primesieve_count_primes(0, 1000);
  printf("Primes below 1000 = %llu\n", count);

  /* use multi-threading for large intervals */
  uint64_t stop = 1000000000;
  count = primesieve_parallel_count_primes(0, stop);
  printf("Primes below 10^9 = %llu\n", count);

  return 0;
}
