///
/// @file   prev_prime1.cpp
/// @brief  Test prev_prime() of primesieve::iterator.
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve.hpp>

#include <stdint.h>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <vector>

void check(bool OK)
{
  std::cout << "   " << (OK ? "OK" : "ERROR") << "\n";
  if (!OK)
    std::exit(1);
}

int main()
{
  std::vector<uint64_t> primes;
  primesieve::generate_primes(100000, &primes);
  primesieve::iterator it;
  uint64_t back = primes.size() - 1;
  uint64_t prime;

  for (uint64_t i = back; i > 0; i--)
  {
    it.jump_to(primes[i]);
    prime = it.prev_prime();
    std::cout << "prev_prime(" << primes[i] << ") = " << prime;
    check(prime == primes[i]);

    it.jump_to(primes[i] - 1);
    prime = it.prev_prime();
    std::cout << "prev_prime(" << primes[i] - 1 << ") = " << prime;
    check(prime == primes[i - 1]);
  }

  it.jump_to(100000000);
  prime = it.prev_prime();
  uint64_t sum = 0;

  // iterate over the primes <= 10^8
  for (; prime > 0; prime = it.prev_prime())
    sum += prime;

  std::cout << "Sum of the primes <= 10^8: " << sum;
  check(sum == 279209790387276ull);

  // Test iterating past the stop_hint
  it.jump_to(100000000, 1000000);
  prime = it.prev_prime();
  sum = 0;

  // iterate over the primes <= 10^8
  for (; prime > 0; prime = it.prev_prime())
    sum += prime;

  std::cout << "Sum of the primes <= 10^8: " << sum;
  check(sum == 279209790387276ull);

  for (uint64_t i = 0; i < 1000; i++)
  {
    prime = it.prev_prime();
    std::cout << "prev_prime(0) = " << prime;
    check(prime == 0);
  }

  for (uint64_t i = 0; i < 1000; i++)
  {
    uint64_t old = prime;
    prime = it.next_prime();
    std::cout << "next_prime(" << old << ") = " << prime;
    check(prime == primes[i]);
  }

  it.jump_to(primes.back() - 1);

  for (uint64_t i = 0; i < 1000; i++)
  {
    prime = it.prev_prime();
    uint64_t p1 = primes.size() - (i + 1);
    uint64_t p2 = primes.size() - (i + 2);
    std::cout << "prev_prime(" << primes[p1] << ") = " << prime;
    check(prime == primes[p2]);
  }

  for (uint64_t i = 0; i < 1000; i++)
  {
    uint64_t old = prime;
    uint64_t j = primes.size() - 1000 + i;
    prime = it.next_prime();
    std::cout << "next_prime(" << old << ") = " << prime;
    check(prime == primes[j]);
  }

  it.jump_to(18446744073709551615ull, 18446744073709551557ull);
  prime = it.prev_prime();
  std::cout << "prev_prime(" << 18446744073709551615ull << ") = " << prime;
  check(prime == 18446744073709551557ull);

  std::cout << std::endl;
  std::cout << "All tests passed successfully!" << std::endl;

  return 0;
}
