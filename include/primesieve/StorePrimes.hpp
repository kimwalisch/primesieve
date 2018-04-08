///
/// @file   StorePrimes.hpp
/// @brief  Store primes in a vector.
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef STOREPRIMES_HPP
#define STOREPRIMES_HPP

#include "iterator.hpp"
#include "pmath.hpp"
#include "primesieve_error.hpp"

#include <stdint.h>
#include <vector>
#include <cstddef>

namespace primesieve {

uint64_t get_max_stop();

template <typename T>
void store_primes(uint64_t start, uint64_t stop, T& primes)
{
  if (start > 0)
    start--;
  if (~stop == 0)
    stop--;

  if (start < stop)
  {
    using V = typename T::value_type;
    std::size_t size = primes.size() + prime_count_approx(start, stop);
    primes.reserve(size);

    primesieve::iterator it(start, stop);
    uint64_t prime = it.next_prime();
    for (; prime <= stop; prime = it.next_prime())
      primes.push_back((V) prime);
  }
}

template <typename T>
void store_n_primes(uint64_t n, uint64_t start, T& primes)
{
  if (n == 0)
    return;
  if (start > 0)
    start--;

  uint64_t stop = get_max_stop();
  std::size_t size = primes.size() + (std::size_t) n;
  primes.reserve(size);
  using V = typename T::value_type;

  primesieve::iterator it(start, stop);
  uint64_t prime = it.next_prime();
  for (; n > 0; n--, prime = it.next_prime())
    primes.push_back((V) prime);

  if (~prime == 0)
    throw primesieve_error("cannot generate primes > 2^64");
}

} // namespace

#endif
