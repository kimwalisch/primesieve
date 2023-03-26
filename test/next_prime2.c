///
/// @file   next_prime2.c
/// @brief  Test primesieve_next_prime().
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
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
  uint64_t old;
  uint64_t prime;
  uint64_t max_prime = primes[size - 1];
  uint64_t sum = 0;

  for (i = 0; i < size - 1; i++)
  {
    primesieve_jump_to(&it, primes[i], max_prime);
    prime = primesieve_next_prime(&it);
    printf("next_prime(%" PRIu64 ") = %" PRIu64, primes[i], prime);
    check(prime == primes[i]);

    primesieve_jump_to(&it, primes[i] + 1, max_prime);
    prime = primesieve_next_prime(&it);
    printf("next_prime(%" PRIu64 ") = %" PRIu64, primes[i] + 1, prime);
    check(prime == primes[i + 1]);
  }

  primesieve_jump_to(&it, 0, max_prime);

  // Iterate over the primes <= 10^9
  while ((prime = primesieve_next_prime(&it)) <= 1000000000)
    sum += prime;

  printf("Sum of the primes <= 10^9: %" PRIu64, sum);
  check(sum == 24739512092254535ull);

  primesieve_jump_to(&it, max_prime / 2, max_prime);
  prime = primesieve_next_prime(&it);

  while (prime <= max_prime)
    prime = primesieve_next_prime(&it);

  for (i = 1; i < 1000; i++)
  {
    old = prime;
    prime = primesieve_prev_prime(&it);
    printf("prev_prime(%" PRIu64 ") = %" PRIu64, old, prime);
    check(prime == primes[size - i]);
  }

  primesieve_jump_to(&it, 18446744073709551556ull, 0);
  prime = primesieve_next_prime(&it);
  printf("next_prime(18446744073709551556) = %" PRIu64, prime);
  check(prime == 18446744073709551557ull);

// This test triggers a GCC bug if GCC version <= 12,
// hence we avoid running this test with GCC <= 12.
// See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=106627
#if __GNUC__ >= 14 || \
    !defined(__GNUC__) || \
    defined(__clang__)

  // Make sure that primesieve_next_prime() returns
  // PRIMESIEVE_ERROR indefinitely when trying to
  // generate primes > 2^64.
  for (i = 0; i < 1000; i++)
  {
    old = prime;
    prime = primesieve_next_prime(&it);
    printf("next_prime(%" PRIu64 ") = PRIMESIEVE_ERROR:", old);
    check(prime == PRIMESIEVE_ERROR);
  }

#endif

  primesieve_free(primes);
  primesieve_free_iterator(&it);

  printf("\n");
  printf("All tests passed successfully!\n");

  return 0;
}
