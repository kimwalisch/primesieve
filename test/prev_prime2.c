///
/// @file   prev_prime2.cpp
/// @brief  Test primesieve_prev_prime().
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
  size_t size = 0;
  uint64_t* primes = (uint64_t*) primesieve_generate_primes(0, 100000, &size, UINT64_PRIMES);
  primesieve_iterator it;
  primesieve_init(&it);

  uint64_t i;
  uint64_t prime;
  uint64_t sum = 0;
  uint64_t old, p1, p2;

  for (i = size - 1; i > 0; i--)
  {
    primesieve_jump_to(&it, primes[i], 0);
    prime = primesieve_prev_prime(&it);
    printf("prev_prime(%" PRIu64 ") = %" PRIu64, primes[i], prime);
    check(prime == primes[i]);

    primesieve_jump_to(&it, primes[i] - 1, 0);
    prime = primesieve_prev_prime(&it);
    printf("prev_prime(%" PRIu64 ") = %" PRIu64, primes[i] - 1, prime);
    check(prime == primes[i - 1]);
  }

  primesieve_jump_to(&it, 100000000, 0);

  // iterate over the primes <= 10^8
  while ((prime = primesieve_prev_prime(&it)) > 0)
    sum += prime;

  printf("Sum of the primes <= 10^8: %" PRIu64, sum);
  check(sum == 279209790387276ull);

  // Test iterating past the stop_hint
  primesieve_jump_to(&it, 100000000, 5000000);
  sum = 0;

  // iterate over the primes <= 10^8
  while ((prime = primesieve_prev_prime(&it)) > 0)
    sum += prime;

  printf("Sum of the primes <= 10^8: %" PRIu64, sum);
  check(sum == 279209790387276ull);

  for (i = 0; i < 1000; i++)
  {
    prime = primesieve_prev_prime(&it);
    printf("prev_prime(0) = %" PRIu64, prime);
    check(prime == 0);
  }

  for (i = 0; i < 1000; i++)
  {
    old = prime;
    prime = primesieve_next_prime(&it);
    printf("next_prime(%" PRIu64 ") = %" PRIu64, old, prime);
    check(prime == primes[i]);
  }

  primesieve_jump_to(&it, primes[size-1] - 1, 0);

  for (i = 0; i < 1000; i++)
  {
    p1 = primes[size - (i + 1)];
    p2 = primes[size - (i + 2)];
    prime = primesieve_prev_prime(&it);
    printf("prev_prime(%" PRIu64 ") = %" PRIu64, p1, prime);
    check(prime == p2);
  }

  for (i = 0; i < 1000; i++)
  {
    old = prime;
    p1 = size - 1000 + i;
    prime = primesieve_next_prime(&it);
    printf("next_prime(%" PRIu64 ") = %" PRIu64, old, prime);
    check(prime == primes[p1]);
  }

  primesieve_free(primes);
  primesieve_free_iterator(&it);

  printf("\n");
  printf("All tests passed successfully!\n");

  return 0;
}
