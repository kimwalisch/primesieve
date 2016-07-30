///
/// @file   primesieve_iterator.cpp
/// @brief  C port of primesieve::iterator.
///
/// Copyright (C) 2016 Kim Walisch, <kim.walisch@gmail.com>
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

/// Get a sieving distance which ensures a good load balance.
/// @param n  Start or stop number.
///
uint64_t get_distance(uint64_t n, uint64_t& tiny_cache_size)
{
  uint64_t cache_size = config::ITERATOR_CACHE_SMALL;

  if (tiny_cache_size < cache_size)
  {
    cache_size = tiny_cache_size;
    tiny_cache_size *= 4;
  }

  n = max<uint64_t>(n, 10);
  double x = static_cast<double>(n);
  double sqrtx = sqrt(x);
  uint64_t primes = static_cast<uint64_t>(sqrtx / (log(sqrtx) - 1));
  uint64_t cache_min_primes = cache_size / sizeof(uint64_t);
  uint64_t cache_max_primes = config::ITERATOR_CACHE_MAX / sizeof(uint64_t);
  primes = inBetween(cache_min_primes, primes, cache_max_primes);
  double distance = primes * log(x);

  return static_cast<uint64_t>(distance);
}

}

/// C constructor
void primesieve_init(primesieve_iterator* pi)
{
  pi->primes_pimpl_ = reinterpret_cast<uint64_t*>(new vector<uint64_t>());
  primesieve_skipto(pi, 0, primesieve_get_max_stop());
}

/// C destructor
void primesieve_free_iterator(primesieve_iterator* pi)
{
  if (pi)
  {
    vector<uint64_t>* primes = &to_vector(pi->primes_pimpl_);
    delete primes;
  }
}

void primesieve_skipto(primesieve_iterator* pi,
                       uint64_t start,
                       uint64_t stop_hint)
{
  vector<uint64_t>& primes = to_vector(pi->primes_pimpl_);
  primes.clear();
  pi->start_ = start;
  pi->stop_ = start;
  pi->stop_hint_ = stop_hint;
  pi->i_ = 0;
  pi->last_idx_ = 0;
  pi->tiny_cache_size_ = 1 << 10;
  pi->is_error_ = false;
}

void primesieve_generate_next_primes(primesieve_iterator* pi)
{
  vector<uint64_t>& primes = to_vector(pi->primes_pimpl_);

  if (!pi->is_error_)
  {
    try
    {
      primes.clear();

      while (primes.empty())
      {
        pi->start_ = add_overflow_safe(pi->stop_, 1);
        pi->stop_ = add_overflow_safe(pi->start_, get_distance(pi->start_, pi->tiny_cache_size_));
        if (pi->start_ <= pi->stop_hint_ && pi->stop_ >= pi->stop_hint_)
          pi->stop_ = add_overflow_safe(pi->stop_hint_, max_prime_gap(pi->stop_hint_));
        generate_primes(pi->start_, pi->stop_, &primes);
        if (primes.empty() && pi->stop_ >= get_max_stop())
          throw primesieve_error("next_prime() > primesieve_get_max_stop()");
      }
    }
    catch (exception&)
    {
      primes.clear();
      primes.resize(64, PRIMESIEVE_ERROR);
      pi->is_error_ = true;
      errno = EDOM;
    }
  }

  pi->primes_ = &primes[0];
  pi->last_idx_ = primes.size() - 1;
  pi->i_ = 0;
}

void primesieve_generate_previous_primes(primesieve_iterator* pi)
{
  vector<uint64_t>& primes = to_vector(pi->primes_pimpl_);

  if (!pi->is_error_)
  {
    try
    {
      primes.clear();

      while (primes.empty())
      {
        pi->stop_ = sub_underflow_safe(pi->start_, 1);
        pi->start_ = sub_underflow_safe(pi->stop_, get_distance(pi->stop_, pi->tiny_cache_size_));
        if (pi->start_ <= pi->stop_hint_ && pi->stop_ >= pi->stop_hint_)
          pi->start_ = sub_underflow_safe(pi->stop_hint_, max_prime_gap(pi->stop_hint_));
        if (pi->start_ <= 2)
          primes.push_back(0);
        generate_primes(pi->start_, pi->stop_, &primes);
      }
    }
    catch (exception&)
    {
      primes.clear();
      primes.resize(64, PRIMESIEVE_ERROR);
      pi->is_error_ = true;
      errno = EDOM;
    }
  }

  pi->primes_ = &primes[0];
  pi->last_idx_ = primes.size() - 1;
  pi->i_ = pi->last_idx_;
}
