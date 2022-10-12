///
/// @file   move_primesieve_iterator.cpp
/// @brief  Test the move constructor and move assignment operators
///         of the primesieve::iterator class.
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve.hpp>
#include <iostream>
#include <memory>
#include <vector>

void check(bool OK)
{
  std::cout << "   " << (OK ? "OK" : "ERROR") << "\n";
  if (!OK)
    std::exit(1);
}

int main()
{
  // test move constructor ///////////////////////////////////////////

  primesieve::iterator it;
  uint64_t prime = it.next_prime();
  uint64_t sum = 0;

  // use 1st iterator up to 5e8
  for (; prime < 500000000ull; prime = it.next_prime())
    sum += prime;

  // move constructor
  primesieve::iterator it2(std::move(it));

  // use 2nd iterator up to 1e9
  for (; prime <= 1000000000ull; prime = it2.next_prime())
    sum += prime;

  std::cout << "Sum of the primes <= 10^9: " << sum;
  check(sum == 24739512092254535ull);

  // test move assignment operator ///////////////////////////////////

  // moved from objects can be reused
  // but they need to be reset
  it.jump_to(0);
  prime = it.next_prime();
  sum = 0;

  // use 1st iterator up to 6e8
  for (; prime < 600000000ull; prime = it.next_prime())
    sum += prime;

  // move assignment operator
  it2 = std::move(it);

  // use 2nd iterator up to 1e9
  for (; prime <= 1000000000ull; prime = it2.next_prime())
    sum += prime;

  std::cout << "Sum of the primes <= 10^9: " << sum;
  check(sum == 24739512092254535ull);

  // test std::vector ////////////////////////////////////////////////

  std::vector<primesieve::iterator> vect;
  vect.emplace_back(1000);
  prime = vect.back().prev_prime();
  std::cout << "1st prime < 1000 = " << prime;
  check(prime == 997);

  it2.jump_to(5);
  vect.emplace_back(std::move(it2));
  prime = vect.back().next_prime();
  std::cout << "1st prime >= 5 = " << prime;
  check(prime == 5);

  return 0;
}
