///
/// @file  iterator.cpp
///
/// Copyright (C) 2015 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/config.hpp>
#include <primesieve/pmath.hpp>
#include <primesieve/PrimeFinder.hpp>
#include <primesieve.hpp>

#include <cmath>
#include <string>
#include <vector>

namespace primesieve {

iterator::iterator(uint64_t start, uint64_t stop_hint)
{
  skipto(start, stop_hint);
}

void iterator::skipto(uint64_t start, uint64_t stop_hint)
{
  if (start > get_max_stop())
    throw primesieve_error("start must be <= " + PrimeFinder::getMaxStopString());

  start_ = start;
  stop_ = start;
  stop_hint_ = stop_hint;
  i_ = 0;
  last_idx_ = 0;
  tiny_cache_size_ = 1 << 11;
  primes_.clear();
}

uint64_t add_overflow_safe(uint64_t a, uint64_t b)
{
  uint64_t max_stop = get_max_stop();
  return (a < max_stop - b) ? a + b : max_stop;
}

uint64_t subtract_underflow_safe(uint64_t a, uint64_t b)
{
  return (a > b) ? a - b : 0;
}

void iterator::generate_next_primes()
{
  primes_.clear();

  while (primes_.empty())
  {
    start_ = add_overflow_safe(stop_, 1);
    stop_ = add_overflow_safe(start_, get_interval_size(start_));
    if (start_ <= stop_hint_ && stop_ >= stop_hint_)
      stop_ = add_overflow_safe(stop_hint_, max_prime_gap(stop_hint_));
    generate_primes(start_, stop_, &primes_);
    if (primes_.empty() && stop_ >= get_max_stop())
      throw primesieve_error("next_prime() > " + PrimeFinder::getMaxStopString());
  }

  last_idx_ = primes_.size() - 1;
  i_ = 0;
}

void iterator::generate_previous_primes()
{
  primes_.clear();

  while (primes_.empty())
  {
    stop_ = subtract_underflow_safe(start_, 1);
    start_ = subtract_underflow_safe(stop_, get_interval_size(stop_));
    if (start_ <= stop_hint_ && stop_ >= stop_hint_)
      start_ = subtract_underflow_safe(stop_hint_, max_prime_gap(stop_hint_));
    if (start_ <= 2)
      primes_.push_back(0);
    generate_primes(start_, stop_, &primes_);
  }

  last_idx_ = primes_.size() - 1;
  i_ = last_idx_;
}

/// Calculate an interval size that ensures a good load balance.
/// @param n  Start or stop number.
///
uint64_t iterator::get_interval_size(uint64_t n)
{
  n = (n > 10) ? n : 10;
  uint64_t cache_size = config::ITERATOR_CACHE_SMALL;

  if (tiny_cache_size_ < cache_size)
  {
    cache_size = tiny_cache_size_;
    tiny_cache_size_ *= 2;
  }

  double x = static_cast<double>(n);
  double sqrtx = std::sqrt(x);
  uint64_t sqrtx_primes = static_cast<uint64_t>(sqrtx / (std::log(sqrtx) - 1));
  uint64_t cache_min_primes = cache_size / sizeof(uint64_t);
  uint64_t cache_max_primes = config::ITERATOR_CACHE_MAX / sizeof(uint64_t);
  uint64_t primes = getInBetween(cache_min_primes, sqrtx_primes, cache_max_primes);

  return static_cast<uint64_t>(primes * std::log(x));
}

} // end namespace
