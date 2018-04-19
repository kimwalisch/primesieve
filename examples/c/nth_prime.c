/** @example nth_prime.c
 *  C program that finds the nth prime. */

#include <primesieve.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>

int main(int argc, char** argv)
{
  uint64_t n = 1000;

  if (argc > 1 && argv[1])
    n = atol(argv[1]);

  uint64_t prime = primesieve_nth_prime(n, 0);
  printf("%" PRIu64 "th prime = %" PRIu64 "\n", n, prime);

  return 0;
}
