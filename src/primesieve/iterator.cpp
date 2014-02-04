///
/// @file  iterator.cpp
///
/// Copyright (C) 2014 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/config.hpp>
#include <primesieve/PrimeFinder.hpp>
#include <primesieve.hpp>

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace {

// log(n)^2 is an approximation of the maximum prime gap near n
uint64_t max_prime_gap(uint64_t n)
{
  double logn = std::log(static_cast<double>(n));
  double prime_gap = logn * logn;
  return static_cast<uint64_t>(prime_gap);
}

}

namespace primesieve {

iterator::iterator(uint64_t start, uint64_t stop_hint)
{
  skipto(start, stop_hint);
}

void iterator::set_stop_hint(uint64_t stop_hint)
{
  stop_hint_ = stop_hint;
}

void iterator::skipto(uint64_t start, uint64_t stop_hint)
{
  if (start_ > get_max_stop())
    throw primesieve_error("start must be <= " + PrimeFinder::getMaxStopString());

  i_ = 0;
  count_ = 0;
  start_ = start;
  stop_ = start;
  set_stop_hint(stop_hint);
  first_ = true;
  is_binary_search_ = (!primes_.empty() &&
                        primes_.front() <= start_ &&
                        primes_.back() >= start_);

  if (!is_binary_search_)
    primes_.clear();
}

void iterator::clear()
{
  primes_.clear();
  skipto(0);
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
  if (is_binary_search_)
  {
    first_ = false;
    is_binary_search_ = false;
    i_ = std::lower_bound(primes_.begin(), primes_.end(), start_) - primes_.begin();
  }
  else
  {
    primes_.clear();
    uint64_t max_stop = get_max_stop();

    while (primes_.empty())
    {
      if (!first_)
        start_ = add_overflow_safe(stop_, 1);
      first_ = false;
      stop_ = add_overflow_safe(start_, get_interval_size(start_));
      if (start_ <= stop_hint_ && stop_ >= stop_hint_)
        stop_ = add_overflow_safe(stop_hint_, max_prime_gap(stop_hint_));
      primesieve::generate_primes(start_, stop_, &primes_);
      if (primes_.empty() && stop_ >= max_stop)
        throw primesieve_error("next_prime() > " + PrimeFinder::getMaxStopString());
    }

    i_ = 0;
  }
}

void iterator::generate_previous_primes()
{
  if (is_binary_search_)
  {
    first_ = false;
    is_binary_search_ = false;
    i_ = std::lower_bound(primes_.begin(), primes_.end(), start_) - primes_.begin();
    if (i_ > 0 && primes_[i_] > start_)
      i_--;
  }
  else
  {
    primes_.clear();

    while (primes_.empty())
    {
      if (!first_)
        stop_ = subtract_underflow_safe(start_, 1);
      first_ = false;
      uint64_t interval_size = get_interval_size(stop_);
      start_ = subtract_underflow_safe(stop_, interval_size);
      if (start_ <= stop_hint_ && stop_ >= stop_hint_)
        start_ = subtract_underflow_safe(stop_hint_, max_prime_gap(stop_hint_));
      primesieve::generate_primes(start_, stop_, &primes_);
      if (primes_.empty() && start_ < 2)
        throw primesieve_error("previous_prime(): smallest prime is 2");
    }

    i_ = primes_.size() - 1;
  }
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
