///
/// @file  iterator.cpp
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/soe/config.hpp>
#include <primesieve/soe/PrimeFinder.hpp>
#include <primesieve.hpp>

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace primesieve {

const std::string max_stop_string = soe::PrimeFinder::getMaxStopString();

iterator::iterator(uint64_t start)
{
  skipto(start);
}

void iterator::skipto(uint64_t start)
{
  first_ = true;
  adjust_skipto_ = false;
  i_ = 0;
  count_ = 0;
  start_ = start;

  if (start_ > max_stop())
    throw primesieve_error("start must be <= " + max_stop_string);

  if (!primes_.empty() &&
       primes_.front() <= start_ &&
       primes_.back() >= start_)
  {
    adjust_skipto_ = true;
    i_ = std::lower_bound(primes_.begin(), primes_.end(), start_) - primes_.begin();
  }
  else
    primes_.clear();
}

void iterator::generate_primes(uint64_t start, uint64_t stop)
{
  primes_.clear();
  primesieve::generate_primes(start, stop, &primes_);
  if (primes_.empty())
  {
    if (stop < 2)
      throw primesieve_error("previous_prime(): smallest prime is 2");
    else
      throw primesieve_error("next_prime() > " + max_stop_string);
  }
}

void iterator::generate_next_primes()
{
  if (adjust_skipto_)
  {
    adjust_skipto_ = false;
    if (i_ > 0 && primes_[i_ - 1] >= start_)
      i_--;
  }
  else
  {
    uint64_t start = (first_) ? start_ : primes_.back() + 1;
    uint64_t interval_size = get_interval_size(start);
    uint64_t stop = (start < max_stop() - interval_size) ? start + interval_size : max_stop();
    generate_primes(start, stop);
    i_ = 0;
  }
  first_ = false;
}

void iterator::generate_previous_primes()
{
  if (adjust_skipto_)
  {
    adjust_skipto_ = false;
    if (i_ > 0 && primes_[i_] > start_)
      i_--;
  }
  else
  {
    uint64_t stop = start_;
    if (!first_)
      stop = (primes_.front() > 1) ? primes_.front() - 1 : 0;
    uint64_t interval_size = get_interval_size(stop);
    uint64_t start = (stop > interval_size) ? stop - interval_size : 0;
    generate_primes(start, stop);
    i_ = primes_.size();
  }
  first_ = false;
}

/// Calculate an interval size that ensures a good load balance.
/// @param n  Start or stop number.
///
uint64_t iterator::get_interval_size(uint64_t n)
{
  using config::ITERATOR_CACHE_SMALL;
  using config::ITERATOR_CACHE_MEDIUM;
  using config::ITERATOR_CACHE_LARGE;

  count_++;
  double x = std::max(static_cast<double>(n), 10.0);
  double sqrtx = std::sqrt(x);
  uint64_t sqrtx_primes = static_cast<uint64_t>(sqrtx / (std::log(sqrtx) - 1));

  uint64_t max_primes = ITERATOR_CACHE_LARGE / sizeof(uint64_t);
  uint64_t primes = ((count_ < 10) ? ITERATOR_CACHE_SMALL : ITERATOR_CACHE_MEDIUM) / sizeof(uint64_t);
  primes = std::min(std::max(primes, sqrtx_primes), max_primes);

  return static_cast<uint64_t>(primes * std::log(x));
}

} // end namespace
