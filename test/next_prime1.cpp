///
/// @file   next_prime1.cpp
/// @brief  Test next_prime() of primesieve::iterator.
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
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

using namespace std;

void check(bool OK)
{
  cout << "   " << (OK ? "OK" : "ERROR") << "\n";
  if (!OK)
    exit(1);
}

int main()
{
  vector<uint64_t> primes;
  primesieve::generate_primes(100000, &primes);
  primesieve::iterator it;
  uint64_t stop = primes.size() - 1;
  uint64_t prime;

  for (uint64_t i = 0; i < stop; i++)
  {
    it.skipto(primes[i] - 1);
    prime = it.next_prime();
    cout << "next_prime(" << primes[i] - 1 << ") = " << prime;
    check(prime == primes[i]);

    it.skipto(primes[i]);
    prime = it.next_prime();
    cout << "next_prime(" << primes[i] << ") = " << prime;
    check(prime == primes[i + 1]);
  }

  it.skipto(0);
  prime = it.next_prime();
  uint64_t sum = 0;

  // iterate over the primes below 10^9
  for (; prime < 1000000000; prime = it.next_prime())
    sum += prime;

  cout << "Sum of the primes below 10^9 = " << sum;
  check(sum == 24739512092254535ull);

  uint64_t p1 = primes[primes.size() - 1];
  uint64_t p2 = primes[primes.size() - 2];
  it.skipto(p1);
  prime = it.prev_prime();
  cout << "prev_prime(" << p1 << ") = " << prime;
  check(prime == p2);

  prime = it.next_prime();
  cout << "next_prime(" << p2 << ") = " << prime;
  check(prime == p1);

  cout << endl;
  cout << "All tests passed successfully!" << endl;

  return 0;
}
