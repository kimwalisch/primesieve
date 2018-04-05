///
/// @file  iterator.cpp
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/config.hpp>
#include <primesieve/NextPrimes.hpp>
#include <primesieve/pmath.hpp>
#include <primesieve.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

using namespace std;
using namespace primesieve;

namespace {

const uint64_t uint64_max = numeric_limits<uint64_t>::max();

void clear(NextPrimes*& ptr)
{
  if (ptr)
    delete ptr;

  ptr = nullptr;
}

bool newNextPrimes(uint64_t prime, uint64_t stop)
{
  return prime == uint64_max &&
          stop != uint64_max;
}

uint64_t get_next_distance(uint64_t n, uint64_t dist)
{
  double x = (double) n;
  x = max(x, 16.0);
  x = sqrt(x) / log(log(x));

  uint64_t min_dist = (uint64_t) x;
  uint64_t limit = uint64_max / 4;
  dist = max(dist, min_dist);

  if (dist < limit)
    dist *= 4;

  return dist;
}

uint64_t get_next_stop(uint64_t start, uint64_t stop_hint, uint64_t* dist)
{
  // check if stop_hint is reasonable
  if (stop_hint >= start &&
      stop_hint < uint64_max)
  {
    uint64_t max_gap = max_prime_gap(stop_hint);
    return checkedAdd(stop_hint, max_gap);
  }

  *dist = get_next_distance(start, *dist);
  return checkedAdd(start, *dist);
}

} // namespace

namespace primesieve {

iterator::iterator(uint64_t start, uint64_t stop_hint)
{
  start_ = start;
  stop_ = start;
  stop_hint_ = stop_hint;
  i_ = 0;
  last_idx_ = 0;
  dist_ = NextPrimes::maxCachedPrime();
  nextPrimes_ = nullptr;
}

void iterator::skipto(uint64_t start, uint64_t stop_hint)
{
  start_ = start;
  stop_ = start;
  stop_hint_ = stop_hint;
  i_ = 0;
  last_idx_ = 0;
  dist_ = NextPrimes::maxCachedPrime();
  primes_.clear();
  clear(nextPrimes_);
}

iterator::~iterator()
{
  clear(nextPrimes_);
}

void iterator::generate_next_primes()
{
  if (!nextPrimes_)
  {
    primes_.resize(64);
    start_ = checkedAdd(stop_, 1);
    stop_ = get_next_stop(start_, stop_hint_, &dist_);
    nextPrimes_ = new NextPrimes(start_, stop_);
  }

  for (last_idx_ = 0; !last_idx_;)
    nextPrimes_->fill(&primes_, &last_idx_);

  if (newNextPrimes(primes_[0], stop_))
  {
    clear(nextPrimes_);
    start_ = checkedAdd(stop_, 1);
    stop_ = get_next_stop(start_, stop_hint_, &dist_);
    nextPrimes_ = new NextPrimes(start_, stop_);

    for (last_idx_ = 0; !last_idx_;)
      nextPrimes_->fill(&primes_, &last_idx_);
  }

  i_ = 0;
  last_idx_--;
}

void iterator::generate_prev_primes()
{
  primes_.clear();
  clear(nextPrimes_);

  while (primes_.empty())
  {
    stop_ = checkedSub(start_, 1);
    start_ = checkedSub(stop_, get_distance(stop_));
    if (start_ <= stop_hint_ && stop_ >= stop_hint_)
      start_ = checkedSub(stop_hint_, max_prime_gap(stop_hint_));
    if (start_ <= 2)
      primes_.push_back(0);
    generate_primes(start_, stop_, &primes_);
  }

  last_idx_ = primes_.size() - 1;
  i_ = last_idx_;
}

/// Get a distance which ensures a good load balance
/// @n:  Start or stop number
///
uint64_t iterator::get_distance(uint64_t n)
{
  n = max<uint64_t>(n, 10);
  uint64_t cache_size = config::MIN_CACHE_ITERATOR;

  if (dist_ < cache_size)
  {
    cache_size = dist_;
    dist_ *= 4;
  }

  double x = (double) n;
  double sqrtx = sqrt(x);
  uint64_t primes = (uint64_t)(sqrtx / (log(sqrtx) - 1));
  uint64_t min_primes = cache_size / sizeof(uint64_t);
  uint64_t max_primes = config::MAX_CACHE_ITERATOR / sizeof(uint64_t);
  primes = inBetween(min_primes, primes, max_primes);
  double distance = primes * log(x);

  return (uint64_t) distance;
}

} // namespace
