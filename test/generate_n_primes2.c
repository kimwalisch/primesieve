///
/// @file   generate_n_primes2.c
/// @brief  Test n prime number generation.
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

// primes inside [0, 100]
const uint64_t small_primes[25] =
{
   2,  3,  5,  7, 11,
  13, 17, 19, 23, 29,
  31, 37, 41, 43, 47,
  53, 59, 61, 67, 71,
  73, 79, 83, 89, 97
};

// primes inside [10^16, 10^16 + 1000]
const uint64_t large_primes[20] =
{
  10000000000000061ull,
  10000000000000069ull,
  10000000000000079ull,
  10000000000000099ull,
  10000000000000453ull,
  10000000000000481ull,
  10000000000000597ull,
  10000000000000613ull,
  10000000000000639ull,
  10000000000000669ull,
  10000000000000753ull,
  10000000000000793ull,
  10000000000000819ull,
  10000000000000861ull,
  10000000000000897ull,
  10000000000000909ull,
  10000000000000931ull,
  10000000000000949ull,
  10000000000000957ull,
  10000000000000991ull,
};

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
  size_t i;
  size_t size = 25;
  uint64_t* primes = (uint64_t*) primesieve_generate_n_primes(size, 0, UINT64_PRIMES);

  for (i = 0; i < size; i++)
  {
    printf("primes[%zu] = %" PRIu64, i, primes[i]);
    check(primes[i] == small_primes[i]);
  }

  primesieve_free(primes);
  size = 20;
  primes = (uint64_t*) primesieve_generate_n_primes(size, 10000000000000000ull, UINT64_PRIMES);

  for (i = 0; i < size; i++)
  {
    printf("primes[%zu] = %" PRIu64, i, primes[i]);
    check(primes[i] == large_primes[i]);
  }

  primesieve_free(primes);

  uint16_t* primes16 = (uint16_t*) primesieve_generate_n_primes(100000, (1 << 16) - 100, UINT16_PRIMES);
  printf("Detect 16-bit overflow:");
  check(primes16 == NULL);

  printf("\n");
  printf("All tests passed successfully!\n");

  return 0;
}
