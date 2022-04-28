///
/// @file   StorePrimes.hpp
/// @brief  Store primes in a vector.
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef STOREPRIMES_HPP
#define STOREPRIMES_HPP

#include "iterator.hpp"
#include "primesieve_error.hpp"

#include <stdint.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace primesieve {

/// primeCountApprox(x) >= pi(x)
inline std::size_t prime_count_approx(uint64_t start, uint64_t stop)
{
  if (start > stop)
    return 0;
  if (stop <= 10)
    return 4;

  // pi(x) <= x / (log(x) - 1.1) + 5, for x >= 4
  double x = (double) stop;
  double logx = std::log(x);
  double pix = (stop - start) / (logx - 1.1) + 5;

  return (std::size_t) pix;
}

template <typename T>
inline void store_primes(uint64_t start,
                         uint64_t stop,
                         T& primes)
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
inline void store_n_primes(uint64_t n,
                           uint64_t start,
                           T& primes)
{
  if (n == 0)
    return;
  if (start > 0)
    start--;

  std::size_t size = primes.size() + (std::size_t) n;
  primes.reserve(size);
  using V = typename T::value_type;

  // nth prime < n(log n + log log n), for n >= 6.
  // https://en.wikipedia.org/wiki/Prime_number_theorem#Approximations_for_the_nth_prime_number
  double logn = std::log(std::max<double>(6.0, (double) n));
  double loglogn = std::log(logn);
  uint64_t dist = (uint64_t)(n * (logn + loglogn));
  uint64_t stop = start + dist;

  primesieve::iterator it(start, stop);
  uint64_t prime = it.next_prime();
  for (; n > 0; n--, prime = it.next_prime())
    primes.push_back((V) prime);

  if (~prime == 0)
    throw primesieve_error("cannot generate primes > 2^64");
}

} // namespace

#endif
