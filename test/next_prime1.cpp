///
/// @file   next_prime1.cpp
/// @brief  Test next_prime() of primesieve::iterator.
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
  uint64_t stop = primes.size() - 1;
  uint64_t prime;

  for (uint64_t i = 0; i < stop; i++)
  {
    it.jump_to(primes[i]);
    prime = it.next_prime();
    std::cout << "next_prime(" << primes[i] << ") = " << prime;
    check(prime == primes[i]);

    it.jump_to(primes[i] + 1);
    prime = it.next_prime();
    std::cout << "next_prime(" << primes[i] + 1 << ") = " << prime;
    check(prime == primes[i + 1]);
  }

  it.jump_to(0);
  prime = it.next_prime();
  uint64_t sum = 0;

  // Iterate over the primes <= 10^9
  for (; prime <= 1000000000; prime = it.next_prime())
    sum += prime;

  std::cout << "Sum of the primes <= 10^9: " << sum;
  check(sum == 24739512092254535ull);

  it.jump_to(primes.back() - 200, primes.back());
  prime = it.next_prime();

  while (prime <= primes.back())
    prime = it.next_prime();

  for (uint64_t i = 1; i < 1000; i++)
  {
    uint64_t old = prime;
    uint64_t p = primes[primes.size() - i];
    prime = it.prev_prime();
    std::cout << "prev_prime(" << old << ") = " << prime;
    check(prime == p);
  }

  std::cout << std::endl;
  std::cout << "All tests passed successfully!" << std::endl;

  return 0;
}
