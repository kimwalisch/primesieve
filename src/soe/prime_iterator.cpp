///
/// @file  prime_iterator.cpp
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve.h>
#include <primesieve/soe/prime_iterator.h>
#include <primesieve/soe/primesieve_error.h>

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

using namespace std;

namespace primesieve {

prime_iterator::prime_iterator(uint64_t start)
{
  skip_to(start);
}

void prime_iterator::skip_to(uint64_t start)
{
  i_ = 0;
  count_ = 0;
  start_ = start;
  first_ = true;
  adjust_skip_to_ = false;

  if (start_ > max_stop())
    throw primesieve_error("start must be <= " + max_stop_string());

  if (!primes_.empty() &&
       primes_.front() <= start_ &&
       primes_.back() >= start_)
  {
    adjust_skip_to_ = true;
    i_ = lower_bound(primes_.begin(), primes_.end(), start_) - primes_.begin();
  }
}

/// Return 0 if out of range, e.g.:
/// prime_iterator pi;
/// pi.skip_to(1);
/// pi.previous_prime() == 0;
///
void prime_iterator::check_out_of_range()
{
  if (primes_.empty())
    primes_.push_back(0);
}

/// Calculate an interval size that ensures a good load balance.
/// @param n  Start or stop number.
///
uint64_t prime_iterator::get_interval_size(uint64_t n)
{
  count_++;
  uint64_t kilobyte = 1 << 10;
  uint64_t megabyte = 1 << 20;
  uint64_t primes = (count_ < 10) ? (kilobyte * 32) / sizeof(uint64_t)
      : (megabyte * 4) / sizeof(uint64_t);

  double dn = max(static_cast<double>(n), 100.0);
  double sqrtn = sqrt(dn);
  uint64_t sqrtn_primes = static_cast<uint64_t>(sqrtn / (log(sqrtn) - 1.0));
  uint64_t max_primes = (megabyte * 512) / sizeof(uint64_t);

  primes = max(primes, sqrtn_primes);
  primes = min(primes, max_primes);
  return static_cast<uint64_t>(primes * log(dn));
}

void prime_iterator::generate_next_primes()
{
  if (adjust_skip_to_)
  {
    adjust_skip_to_ = false;
    if (i_ + 1 < primes_.size() && primes_[i_] < start_)
      i_++;
  }
  else
  {
    uint64_t start = (first_) ? start_ : primes_.back() + 1;
    uint64_t interval_size = get_interval_size(start);
    uint64_t stop = (start < max_stop() - interval_size) ? start + interval_size : max_stop();
    primes_.clear();
    generate_primes(start, stop, &primes_);
    check_out_of_range();
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
    primes_.clear();
    generate_primes(start, stop, &primes_);
    check_out_of_range();
    i_ = primes_.size();
  }
  first_ = false;
}

} // end namespace
