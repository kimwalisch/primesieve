///
/// @file   iterator-c.cpp
/// @brief  C port of primesieve::iterator.
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/config.hpp>
#include <primesieve/pmath.hpp>
#include <primesieve.hpp>
#include <primesieve.h>

#include <algorithm>
#include <exception>
#include <cerrno>
#include <cmath>
#include <vector>

using namespace std;
using namespace primesieve;

namespace {

/// Convert pimpl pointer to vector
vector<uint64_t>& to_vector(uint64_t* primes_pimpl)
{
  vector<uint64_t>* primes = reinterpret_cast<vector<uint64_t>*>(primes_pimpl);
  return *primes;
}

/// Get a distance which ensures a good load balance.
/// @param n  Start or stop number.
///
uint64_t get_distance(uint64_t n, uint64_t& tiny_cache_size)
{
  n = max<uint64_t>(n, 10);
  uint64_t cache_size = config::ITERATOR_CACHE_SMALL;

  if (tiny_cache_size < cache_size)
  {
    cache_size = tiny_cache_size;
    tiny_cache_size *= 4;
  }

  double x = (double) n;
  double sqrtx = sqrt(x);
  uint64_t primes = (uint64_t)(sqrtx / (log(sqrtx) - 1));
  uint64_t cache_min_primes = cache_size / sizeof(uint64_t);
  uint64_t cache_max_primes = config::ITERATOR_CACHE_MAX / sizeof(uint64_t);
  primes = inBetween(cache_min_primes, primes, cache_max_primes);
  double distance = primes * log(x);

  return (uint64_t) distance;
}

}

/// C constructor
void primesieve_init(primesieve_iterator* it)
{
  it->primes_pimpl_ = reinterpret_cast<uint64_t*>(new vector<uint64_t>());
  primesieve_skipto(it, 0, primesieve_get_max_stop());
}

/// C destructor
void primesieve_free_iterator(primesieve_iterator* it)
{
  if (it)
  {
    vector<uint64_t>* primes = &to_vector(it->primes_pimpl_);
    delete primes;
  }
}

void primesieve_skipto(primesieve_iterator* it,
                       uint64_t start,
                       uint64_t stop_hint)
{
  vector<uint64_t>& primes = to_vector(it->primes_pimpl_);
  primes.clear();
  it->start_ = start;
  it->stop_ = start;
  it->stop_hint_ = stop_hint;
  it->i_ = 0;
  it->last_idx_ = 0;
  it->tiny_cache_size_ = 1 << 10;
  it->is_error_ = false;
}

void primesieve_generate_next_primes(primesieve_iterator* it)
{
  vector<uint64_t>& primes = to_vector(it->primes_pimpl_);

  if (!it->is_error_)
  {
    try
    {
      primes.clear();

      while (primes.empty())
      {
        it->start_ = checkedAdd(it->stop_, 1);
        it->stop_ = checkedAdd(it->start_, get_distance(it->start_, it->tiny_cache_size_));
        if (it->start_ <= it->stop_hint_ && it->stop_ >= it->stop_hint_)
          it->stop_ = checkedAdd(it->stop_hint_, max_prime_gap(it->stop_hint_));
        generate_primes(it->start_, it->stop_, &primes);
        if (primes.empty() && it->stop_ >= get_max_stop())
          throw primesieve_error("next_prime() > primesieve_get_max_stop()");
      }
    }
    catch (exception&)
    {
      primes.clear();
      primes.resize(64, PRIMESIEVE_ERROR);
      it->is_error_ = true;
      errno = EDOM;
    }
  }

  it->primes_ = &primes[0];
  it->last_idx_ = primes.size() - 1;
  it->i_ = 0;
}

void primesieve_generate_prev_primes(primesieve_iterator* it)
{
  vector<uint64_t>& primes = to_vector(it->primes_pimpl_);

  if (!it->is_error_)
  {
    try
    {
      primes.clear();

      while (primes.empty())
      {
        it->stop_ = checkedSub(it->start_, 1);
        it->start_ = checkedSub(it->stop_, get_distance(it->stop_, it->tiny_cache_size_));
        if (it->start_ <= it->stop_hint_ && it->stop_ >= it->stop_hint_)
          it->start_ = checkedSub(it->stop_hint_, max_prime_gap(it->stop_hint_));
        if (it->start_ <= 2)
          primes.push_back(0);
        generate_primes(it->start_, it->stop_, &primes);
      }
    }
    catch (exception&)
    {
      primes.clear();
      primes.resize(64, PRIMESIEVE_ERROR);
      it->is_error_ = true;
      errno = EDOM;
    }
  }

  it->primes_ = &primes[0];
  it->last_idx_ = primes.size() - 1;
  it->i_ = it->last_idx_;
}
