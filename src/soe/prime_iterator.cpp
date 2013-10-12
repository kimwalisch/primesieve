///
/// @file  prime_iterator.cpp
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve.h>

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace primesieve {

std::string max_stop_string();

prime_iterator::prime_iterator(uint64_t start)
{
  skip_to(start);
}

void prime_iterator::skip_to(uint64_t start)
{
  first_ = true;
  adjust_skip_to_ = false;
  i_ = 0;
  count_ = 0;
  start_ = start;

  if (start_ > max_stop())
    throw primesieve_error("start must be <= " + max_stop_string());

  if (!primes_.empty() &&
       primes_.front() <= start_ &&
       primes_.back() >= start_)
  {
    adjust_skip_to_ = true;
    i_ = std::lower_bound(primes_.begin(), primes_.end(), start_) - primes_.begin();
  }
}

void prime_iterator::generate_primes(uint64_t start, uint64_t stop)
{
  primes_.clear();
  primesieve::generate_primes(start, stop, &primes_);
  if (primes_.empty())
    primes_.push_back(0);
}

void prime_iterator::generate_next_primes()
{
  if (adjust_skip_to_)
  {
    adjust_skip_to_ = false;
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

void prime_iterator::generate_previous_primes()
{
  if (adjust_skip_to_)
  {
    adjust_skip_to_ = false;
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
uint64_t prime_iterator::get_interval_size(uint64_t n)
{
  count_++;
  const uint64_t KILOBYTE = 1 << 10;
  const uint64_t MEGABYTE = 1 << 20;

  double x = std::max(static_cast<double>(n), 10.0);
  double sqrtx = std::sqrt(x);
  uint64_t sqrtx_primes = static_cast<uint64_t>(sqrtx / (std::log(sqrtx) - 1));

  uint64_t max_primes = (MEGABYTE * 512) / sizeof(uint64_t);
  uint64_t primes = ((count_ < 10) ? (KILOBYTE * 32) : (MEGABYTE * 4)) / sizeof(uint64_t);
  primes = std::min(std::max(primes, sqrtx_primes), max_primes);

  return static_cast<uint64_t>(primes * std::log(x));
}

} // end namespace
