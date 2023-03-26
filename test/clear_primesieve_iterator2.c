///
/// @file   clear_primesieve_iterator2.c
/// @brief  Test next_prime() of primesieve::iterator.
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve.h>

#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

void check(int OK)
{
  if (OK)
    printf("   OK\n");
  else
  {
    printf("   ERROR\n");
    exit(1);
  }
}

int main(void)
{
  primesieve_iterator it;
  primesieve_init(&it);
  uint64_t primes = 0;

  for (int i = 0; i < 10; i++)
  {
    primesieve_clear(&it);
    uint64_t prime = primesieve_next_prime(&it);
    for (; prime < 100000; prime = primesieve_next_prime(&it))
      primes++;
  }

  printf("Count of the primes = %" PRIu64, primes);
  check(primes == 9592 * 10);

  primesieve_free_iterator(&it);
  printf("\n");
  printf("All tests passed successfully!\n");

  return 0;
}
