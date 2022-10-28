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
#include <limits>
#include <string>

namespace primesieve {

/// primeCountApprox(x) >= pi(x)
inline std::size_t prime_count_approx(uint64_t start, uint64_t stop)
{
  if (start > stop)
    return 0;

  // pi(x) <= x / (log(x) - 1.1) + 5, for x >= 4.
  // Pierre Dusart, https://arxiv.org/abs/1002.0442 eq. 6.6.
  double x = (double) stop;
  x = std::max<double>(100.0, x);
  double pix = (stop - start) / (std::log(x) - 1.1) + 5;

  return (std::size_t) pix;
}

template <typename T>
inline void store_primes(uint64_t start,
                         uint64_t stop,
                         T& primes)
{
  if (~stop == 0)
    stop--;
  if (start > stop)
    return;

  using V = typename T::value_type;
  if (stop > std::numeric_limits<V>::max())
    throw primesieve_error("store_primes(): Type is too narrow for generating primes up to " + std::to_string(stop));

  std::size_t size = primes.size() + prime_count_approx(start, stop);
  primes.reserve(size);

  primesieve::iterator it(start, stop);
  it.generate_next_primes();

#if defined(_MSC_VER)
  // Disable warning: conversion from X to Y, possible loss of data
  #pragma warning(push)
  #pragma warning(disable : 4244)
#endif

  for (; it.primes_[it.size_ - 1] <= stop; it.generate_next_primes())
    primes.insert(primes.end(), it.primes_, it.primes_ + it.size_);
  for (std::size_t i = 0; it.primes_[i] <= stop; i++)
    primes.push_back((V) it.primes_[i]);

#if defined(_MSC_VER)
  #pragma warning(pop)
#endif
}

template <typename T>
inline void store_n_primes(uint64_t n,
                           uint64_t start,
                           T& primes)
{
  if (n == 0)
    return;

  using V = typename T::value_type;
  std::size_t size = primes.size() + (std::size_t) n;
  primes.reserve(size);

  // nthPrime < n(log n + log log n), for n >= 6.
  // https://en.wikipedia.org/wiki/Prime_number_theorem#Approximations_for_the_nth_prime_number
  double x = std::max<double>({6.0, (double) n, (double) start});
  double logn = std::log(x);
  double loglogn = std::log(logn);
  uint64_t nthPrime = (uint64_t)(n * (logn + loglogn));
  uint64_t stop = start + nthPrime;

  primesieve::iterator it(start, stop);
  it.generate_next_primes();

#if defined(_MSC_VER)
  // Disable warning: conversion from X to Y, possible loss of data
  #pragma warning(push)
  #pragma warning(disable : 4244)
#endif

  while (n >= it.size_)
  {
    if (it.primes_[0] == std::numeric_limits<uint64_t>::max())
      throw primesieve_error("store_n_primes(): Cannot generate primes > 2^64");
    if (it.primes_[it.size_ - 1] > std::numeric_limits<V>::max())
      throw primesieve_error("store_n_primes(): Type is too narrow for generating primes up to " + std::to_string(stop));

    primes.insert(primes.end(), it.primes_, it.primes_ + it.size_);
    n -= it.size_;
    if (n == 0)
      return;

    it.generate_next_primes();
  }

  if (it.primes_[0] == std::numeric_limits<uint64_t>::max())
    throw primesieve_error("store_n_primes(): Cannot generate primes > 2^64");
  if (it.primes_[n - 1] > std::numeric_limits<V>::max())
    throw primesieve_error("store_n_primes(): Type is too narrow for generating primes up to " + std::to_string(stop));

  for (std::size_t i = 0; i < (std::size_t) n; i++)
    primes.push_back((V) it.primes_[i]);

#if defined(_MSC_VER)
  #pragma warning(pop)
#endif
}

} // namespace

#endif
