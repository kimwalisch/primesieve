///
/// @file  iterator.cpp
///
/// Copyright (C) 2016 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/config.hpp>
#include <primesieve/pmath.hpp>
#include <primesieve/PrimeFinder.hpp>
#include <primesieve.hpp>

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

using namespace std;

namespace primesieve {

iterator::iterator(uint64_t start, uint64_t stop_hint)
{
  skipto(start, stop_hint);
}

void iterator::skipto(uint64_t start, uint64_t stop_hint)
{
  start_ = start;
  stop_ = start;
  stop_hint_ = stop_hint;
  i_ = 0;
  last_idx_ = 0;
  tiny_cache_size_ = 1 << 10;
  primes_.clear();
}

void iterator::generate_next_primes()
{
  primes_.clear();

  while (primes_.empty())
  {
    start_ = add_overflow_safe(stop_, 1);
    stop_ = add_overflow_safe(start_, get_distance(start_));
    if (start_ <= stop_hint_ && stop_ >= stop_hint_)
      stop_ = add_overflow_safe(stop_hint_, max_prime_gap(stop_hint_));
    generate_primes(start_, stop_, &primes_);
    if (primes_.empty() &&
        stop_ >= get_max_stop())
      throw primesieve_error("next_prime() > 2^64");
  }

  last_idx_ = primes_.size() - 1;
  i_ = 0;
}

void iterator::generate_previous_primes()
{
  primes_.clear();

  while (primes_.empty())
  {
    stop_ = sub_underflow_safe(start_, 1);
    start_ = sub_underflow_safe(stop_, get_distance(stop_));
    if (start_ <= stop_hint_ && stop_ >= stop_hint_)
      start_ = sub_underflow_safe(stop_hint_, max_prime_gap(stop_hint_));
    if (start_ <= 2)
      primes_.push_back(0);
    generate_primes(start_, stop_, &primes_);
  }

  last_idx_ = primes_.size() - 1;
  i_ = last_idx_;
}

/// Get a sieving distance which ensures a good load balance.
/// @param n  Start or stop number.
///
uint64_t iterator::get_distance(uint64_t n)
{
  uint64_t cache_size = config::ITERATOR_CACHE_SMALL;

  if (tiny_cache_size_ < cache_size)
  {
    cache_size = tiny_cache_size_;
    tiny_cache_size_ *= 4;
  }

  n = max<uint64_t>(n, 10);
  double x = static_cast<double>(n);
  double sqrtx = sqrt(x);
  uint64_t primes = static_cast<uint64_t>(sqrtx / (log(sqrtx) - 1));
  uint64_t cache_min_primes = cache_size / sizeof(uint64_t);
  uint64_t cache_max_primes = config::ITERATOR_CACHE_MAX / sizeof(uint64_t);
  primes = inBetween(cache_min_primes, primes, cache_max_primes);
  double distance = primes * log(x);

  return static_cast<uint64_t>(distance);
}

} // namespace
