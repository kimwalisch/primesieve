///
/// @file   primesieve_iterator.cpp
/// @brief  C port of primesieve::iterator.
///
/// Copyright (C) 2014 Kim Walisch, <kim.walisch@gmail.com>
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

using namespace primesieve;

namespace {

/// Convert pimpl pointer to std::vector
std::vector<uint64_t>& to_vector(uint64_t* primes_pimpl)
{
  std::vector<uint64_t>* primes = reinterpret_cast<std::vector<uint64_t>*>(primes_pimpl);
  return *primes;
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

/// Calculate an interval size that ensures a good load balance.
/// @param n  Start or stop number.
///
uint64_t get_interval_size(uint64_t n, uint64_t& tiny_cache_size)
{
  n = (n > 10) ? n : 10;
  uint64_t cache_size = config::ITERATOR_CACHE_SMALL;
  if (tiny_cache_size < cache_size)
  {
    cache_size = tiny_cache_size;
    tiny_cache_size *= 2;
  }

  double x = static_cast<double>(n);
  double sqrtx = std::sqrt(x);
  uint64_t sqrtx_primes = static_cast<uint64_t>(sqrtx / (std::log(sqrtx) - 1));
  uint64_t cache_primes = cache_size / sizeof(uint64_t);
  uint64_t cache_max_primes = config::ITERATOR_CACHE_MAX / sizeof(uint64_t);
  uint64_t primes = std::min(std::max(cache_primes, sqrtx_primes), cache_max_primes);

  return static_cast<uint64_t>(primes * std::log(x));
}

}

/// C constructor
void primesieve_init(primesieve_iterator* pi)
{
  pi->primes_pimpl_ = reinterpret_cast<uint64_t*>(new std::vector<uint64_t>);
  primesieve_skipto(pi, 0, primesieve_get_max_stop());
}

/// C destructor
void primesieve_free_iterator(primesieve_iterator* pi)
{
  if (pi)
  {
    std::vector<uint64_t>* primes = &to_vector(pi->primes_pimpl_);
    delete primes;
  }
}

void primesieve_skipto(primesieve_iterator* pi, uint64_t start, uint64_t stop_hint)
{
  std::vector<uint64_t>& primes = to_vector(pi->primes_pimpl_);
  primes.clear();
  pi->start_ = start;
  pi->stop_ = start;
  pi->stop_hint_ = stop_hint;
  pi->i_ = 0;
  pi->last_idx_ = 0;
  pi->tiny_cache_size_ = 1 << 11;
  pi->is_error_ = false;
}

void primesieve_generate_next_primes(primesieve_iterator* pi)
{
  std::vector<uint64_t>& primes = to_vector(pi->primes_pimpl_);

  if (!pi->is_error_)
  {
    try
    {
      primes.clear();

      while (primes.empty())
      {
        pi->start_ = add_overflow_safe(pi->stop_, 1);
        pi->stop_ = add_overflow_safe(pi->start_, get_interval_size(pi->start_, pi->tiny_cache_size_));
        if (pi->start_ <= pi->stop_hint_ && pi->stop_ >= pi->stop_hint_)
          pi->stop_ = add_overflow_safe(pi->stop_hint_, max_prime_gap(pi->stop_hint_));
        primesieve::generate_primes(pi->start_, pi->stop_, &primes);
        if (primes.empty() && pi->stop_ >= get_max_stop())
          throw primesieve_error("next_prime() > primesieve_get_max_stop()");
      }
    }
    catch (std::exception&)
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
  std::vector<uint64_t>& primes = to_vector(pi->primes_pimpl_);

  if (!pi->is_error_)
  {
    try
    {
      primes.clear();

      while (primes.empty())
      {
        pi->stop_ = subtract_underflow_safe(pi->start_, 1);
        pi->start_ = subtract_underflow_safe(pi->stop_, get_interval_size(pi->stop_, pi->tiny_cache_size_));
        if (pi->start_ <= pi->stop_hint_ && pi->stop_ >= pi->stop_hint_)
          pi->start_ = subtract_underflow_safe(pi->stop_hint_, max_prime_gap(pi->stop_hint_));
        primesieve::generate_primes(pi->start_, pi->stop_, &primes);
        if (primes.empty() && pi->start_ < 2)
          throw primesieve_error("previous_prime(): smallest prime is 2");
      }
    }
    catch (std::exception&)
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
