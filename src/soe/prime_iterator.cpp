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
#include <primesieve/soe/PrimeFinder.h>

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

using soe::PrimeFinder;

namespace {

// MAX_STOP = 2^64 - 2^32 * 10
const uint64_t MAX_STOP = PrimeFinder::getMaxStop();

/// Calculate an interval size that ensures a good load balance.
/// @param n  start or stop number.
///
uint64_t get_interval_size(uint64_t n)
{
  const uint64_t MEGABYTE = 1 << 20;
  double dn = std::max(static_cast<double>(n), 100.0);
  double sqrtn = std::sqrt(dn);
  uint64_t primes = static_cast<uint64_t>(sqrtn / (std::log(sqrtn) - 1.0));

  // lower limit = 2 megabytes
  if (primes * sizeof(uint64_t) < MEGABYTE * 2)
   primes = MEGABYTE * 2 / sizeof(uint64_t);
  // upper limit = 1 gigabyte
  if (primes * sizeof(uint64_t) > MEGABYTE * 512)
   primes = MEGABYTE * 512 / sizeof(uint64_t);

  uint64_t interval_size = static_cast<uint64_t>(primes * std::log(dn));
  return interval_size;
}

} // end namespace

namespace primesieve {

prime_iterator::prime_iterator(uint64_t start)
{
  skip_to(start);
}

void prime_iterator::skip_to(uint64_t start)
{
  start_ = start;
  first_ = true;
  adjust_skip_to_ = false;

  if (start_ > MAX_STOP)
    throw primesieve_error("start must be <= " + PrimeFinder::getMaxStopString());

  if (!primes_.empty() &&
       primes_.front() <= start_ &&
       primes_.back() >= start_)
  {
    adjust_skip_to_ = true;
    i_ = std::lower_bound(primes_.begin(), primes_.end(), start_)
        - primes_.begin();
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

/// Generate new next primes.
/// @return  next prime.
///
uint64_t prime_iterator::generate_next_primes()
{
  if (adjust_skip_to_)
  {
    adjust_skip_to_ = false;
    if (i_ + 1 < primes_.size() && primes_[i_] < start_)
      i_++;
  }
  else
  {
    uint64_t start = start_;
    if (!first_)
      start = primes_.back() + 1;
    uint64_t interval_size = get_interval_size(start);
    uint64_t stop = MAX_STOP;
    if (start < MAX_STOP - interval_size)
      stop = start + interval_size;
    primes_.clear();
    generate_primes(start, stop, &primes_);
    check_out_of_range();
    i_ = 0;
  }
  first_ = false;
  size_ = primes_.size();
  return primes_[i_];
}

/// Generate new previous primes.
/// @return  previous prime.
///
uint64_t prime_iterator::generate_previous_primes()
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
    uint64_t start = 0;
    if (stop > interval_size)
      start = stop - interval_size;
    primes_.clear();
    generate_primes(start, stop, &primes_);
    check_out_of_range();
    i_ = primes_.size() - 1;
  }
  first_ = false;
  size_ = primes_.size();
  return primes_[i_];
}

} // end namespace
