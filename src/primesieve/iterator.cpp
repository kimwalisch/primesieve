///
/// @file  iterator.cpp
///
/// Copyright (C) 2014 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/config.hpp>
#include <primesieve/imath.hpp>
#include <primesieve/PrimeFinder.hpp>
#include <primesieve.hpp>

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace primesieve {

iterator::iterator(uint64_t start, uint64_t stop_hint)
{
  skipto(start, stop_hint);
}

void iterator::set_stop_hint(uint64_t stop_hint)
{
  stop_hint_ = stop_hint;
}

void iterator::clear()
{
  skipto(0);
}

void iterator::skipto(uint64_t start, uint64_t stop_hint)
{
  if (start_ > get_max_stop())
    throw primesieve_error("start must be <= " + PrimeFinder::getMaxStopString());

  start_ = start;
  stop_ = start;
  stop_hint_ = stop_hint;
  i_ = 0;
  last_idx_ = 0;
  count_ = 0;
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
  bool first = primes_.empty();
  uint64_t max_stop = get_max_stop();
  primes_.clear();

  while (primes_.empty())
  {
    if (!first)
      start_ = add_overflow_safe(stop_, 1);
    first = false;
    stop_ = add_overflow_safe(start_, get_interval_size(start_));
    if (start_ <= stop_hint_ && stop_ >= stop_hint_)
      stop_ = add_overflow_safe(stop_hint_, max_prime_gap(stop_hint_));
    primesieve::generate_primes(start_, stop_, &primes_);
    if (primes_.empty() && stop_ >= max_stop)
      throw primesieve_error("next_prime() > " + PrimeFinder::getMaxStopString());
  }

  last_idx_ = primes_.size() - 1;
  i_ = 0;
}

void iterator::generate_previous_primes()
{
  bool first = primes_.empty();
  primes_.clear();

  while (primes_.empty())
  {
    if (!first)
      stop_ = subtract_underflow_safe(start_, 1);
    first = false;
    uint64_t interval_size = get_interval_size(stop_);
    start_ = subtract_underflow_safe(stop_, interval_size);
    if (start_ <= stop_hint_ && stop_ >= stop_hint_)
      start_ = subtract_underflow_safe(stop_hint_, max_prime_gap(stop_hint_));
    primesieve::generate_primes(start_, stop_, &primes_);
    if (primes_.empty() && start_ < 2)
      throw primesieve_error("previous_prime(): smallest prime is 2");
  }

  last_idx_ = primes_.size() - 1;
  i_ = last_idx_;
}

/// Calculate an interval size that ensures a good load balance.
/// @param n  Start or stop number.
///
uint64_t iterator::get_interval_size(uint64_t n)
{
  count_++;
  double x = std::max(static_cast<double>(n), 10.0);
  double sqrtx = std::sqrt(x);
  uint64_t sqrtx_primes = static_cast<uint64_t>(sqrtx / (std::log(sqrtx) - 1));

  uint64_t cache_max_primes = config::ITERATOR_CACHE_LARGE / sizeof(uint64_t);
  uint64_t cache_bytes = (count_ < 10) ? 1024 << count_ : config::ITERATOR_CACHE_MEDIUM;
  uint64_t cache_primes = cache_bytes / sizeof(uint64_t);
  uint64_t primes = std::min(std::max(cache_primes, sqrtx_primes), cache_max_primes);

  return static_cast<uint64_t>(primes * std::log(x));
}

} // end namespace
