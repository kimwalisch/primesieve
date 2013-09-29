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

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <vector>

namespace {

enum {
  MEGABYTE = 1 << 20
};

/// Estimate interval size based on number of primes.
/// @param n       start or stop number.
/// @param primes  Number of primes to generate.
///
uint64_t get_interval_size(uint64_t n, std::size_t primes)
{
  const uint64_t MIN_INTERVAL = 1 << 16;
  n = std::max(n, static_cast<uint64_t>(primes) * 20);
  double dn = static_cast<double>(n);
  double logn = std::log(dn);
  uint64_t interval_size = static_cast<uint64_t>(primes * logn);
  return std::max(MIN_INTERVAL, interval_size);
}

} // end namespace

namespace primesieve {

prime_iterator::prime_iterator(uint64_t start, std::size_t cache_size) :
  i_(0),
  cache_size_(cache_size),
  start_(start),
  first_(true)
{
  if (cache_size_ < 1)
    cache_size_ = 1;
  if (cache_size_ > 1024)
    cache_size_ = 1024;

  max_size_ = (cache_size_ * MEGABYTE) / sizeof(uint64_t);
  uint64_t interval_size = get_interval_size(start_, max_size_);
  uint64_t mid = interval_size / 2;
  uint64_t begin = (start_ > mid) ? start_ - mid : 0;

  generate_n_primes(max_size_, begin, &primes_);
  size_ = primes_.size();
  i_ = std::lower_bound(primes_.begin(), primes_.end(), start_) - primes_.begin();
}

std::size_t prime_iterator::get_cache_size() const
{
  return cache_size_;
}

uint64_t prime_iterator::first_prime()
{
  // undefined behavior
  if (primes_[i_] != start_)
    return 0;

  first_ = false;
  return primes_[i_];
}

uint64_t prime_iterator::first_next_prime()
{
  first_ = false;
  if (primes_[i_] < start_)
    i_++;
  return primes_[i_];
}

uint64_t prime_iterator::first_previous_prime()
{
  first_ = false;
  if (primes_[i_] > start_)
    i_--;
  return primes_[i_];
}

void prime_iterator::generate_next_primes()
{
  uint64_t start = primes_.back() + 1;
  primes_.clear();
  generate_n_primes(max_size_, start, &primes_);
  size_ = primes_.size();
  i_ = 0;
}

void prime_iterator::generate_previous_primes()
{
  uint64_t stop = primes_.front();
  if (stop > 1)
    stop--;

  uint64_t interval_size = get_interval_size(stop, max_size_);
  uint64_t start = 0;
  if (stop > interval_size)
    start = stop - interval_size;

  primes_.clear();
  generate_primes(start, stop, &primes_);

  if (primes_.empty())
    primes_.push_back(0);
  i_ = size_ = primes_.size();
}

} // end namespace
