/** @example nth_prime.c
 *  C program that finds the nth prime. */

#include <primesieve.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
  uint64_t n = 1000;
  if (argv[1])
    n = atol(argv[1]);

  uint64_t prime = primesieve_nth_prime(n, 0);
  printf("%lluth prime = %llu\n", n, prime);

  return 0;
}
