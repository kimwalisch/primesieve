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
#include <cmath>
#include <vector>

namespace {

uint64_t megabytes(uint64_t n)
{
  return n << 20;
}

/// Estimate interval size based on number of primes.
/// @param n       start or stop number.
/// @param primes  Number of primes to generate.
///
uint64_t get_interval_size(uint64_t n, uint64_t primes)
{
  const uint64_t MIN_INTERVAL = 1 << 16;
  n = std::max(n, primes * 20);
  double dn = static_cast<double>(n);
  double logn = std::log(dn);
  uint64_t interval_size = static_cast<uint64_t>(primes * logn);
  return std::max(MIN_INTERVAL, interval_size);
}

} // end namespace

namespace primesieve {

prime_iterator::prime_iterator(uint64_t start) :
  start_(start),
  first_(true)
{
  set_cache_size(start_);
  uint64_t interval_size = get_interval_size(start_, cache_size_);
  uint64_t mid = interval_size / 2;
  uint64_t begin = (start_ > mid) ? start_ - mid : 0;

  generate_n_primes(cache_size_, begin, &primes_);
  cache_size_ = primes_.size();
  i_ = std::lower_bound(primes_.begin(), primes_.end(), start_) - primes_.begin();
}

/// Each time new primes have to be generated the cache
/// size is dynamically grown or shrunk.
///
void prime_iterator::set_cache_size(uint64_t n)
{
  // estimate number of primes <= sqrt(n)
  double dn = std::max(static_cast<double>(n), 100.0);
  cache_size_ = static_cast<uint64_t>(std::sqrt(dn) / (std::log(dn) - 1.0));

  // lower limit = 2 megabytes
  if (cache_size_ * sizeof(uint64_t) < megabytes(2))
   cache_size_ = megabytes(2) / sizeof(uint64_t);

  // upper limit = 1 gigabyte
  if (cache_size_ * sizeof(uint64_t) > megabytes(1024))
   cache_size_ = megabytes(1024) / sizeof(uint64_t);
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
  set_cache_size(primes_.back());
  uint64_t start = primes_.back() + 1;
  primes_.clear();
  generate_n_primes(cache_size_, start, &primes_);
  cache_size_ = primes_.size();
  i_ = 0;
}

void prime_iterator::generate_previous_primes()
{
  uint64_t stop = primes_.front();
  if (stop > 1)
    stop--;

  set_cache_size(primes_.front());
  uint64_t interval_size = get_interval_size(stop, cache_size_);
  uint64_t start = 0;
  if (stop > interval_size)
    start = stop - interval_size;

  primes_.clear();
  generate_primes(start, stop, &primes_);

  if (primes_.empty())
    primes_.push_back(0);
  i_ = cache_size_ = primes_.size();
}

} // end namespace
