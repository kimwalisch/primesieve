///
/// @file   primesieve_iterator-c.cpp
/// @brief  C binding primesieve iterator.
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve.h>
#include <primesieve-c.h>

#include <algorithm>
#include <cerrno>
#include <cmath>
#include <vector>
#include <limits>

namespace {

/// Convert raw pointer to std::vector
std::vector<uint64_t>& to_vector(uint64_t* primes_pimpl)
{
  std::vector<uint64_t>* primes = reinterpret_cast<std::vector<uint64_t>*>(primes_pimpl);
  return *primes;
}

/// Generate the primes inside [start, stop] and
/// store them in the primes vector.
///
void generate_primes(primesieve_iterator* pi, uint64_t start, uint64_t stop)
{
  std::vector<uint64_t>& primes = to_vector(pi->primes_pimpl_);
  primes.clear();
  primesieve::generate_primes(start, stop, &primes);
  if (primes.empty())
  {
    uint64_t ERROR = std::numeric_limits<uint64_t>::max();
    primes.resize(1024, ERROR);
    errno = EDOM;
  }
  pi->primes_ = &primes.front();
  pi->size_ = primes.size();
}

/// Calculate an interval size that ensures a good load balance.
/// @param n  Start or stop number.
///
uint64_t get_interval_size(primesieve_iterator* pi, uint64_t n)
{
  pi->count_++;
  uint64_t KILOBYTE = 1 << 10;
  uint64_t MEGABYTE = 1 << 20;

  double x = std::max(static_cast<double>(n), 10.0);
  double sqrtx = std::sqrt(x);
  uint64_t sqrtx_primes = static_cast<uint64_t>(sqrtx / (std::log(sqrtx) - 1));

  uint64_t max_primes = (MEGABYTE * 512) / sizeof(uint64_t);
  uint64_t primes = ((pi->count_ < 10) ? (KILOBYTE * 32) : (MEGABYTE * 4)) / sizeof(uint64_t);
  primes = std::min(std::max(primes, sqrtx_primes), max_primes);

  return static_cast<uint64_t>(primes * std::log(x));
}

} // end namespace

void primesieve_skipto(primesieve_iterator* pi, uint64_t start)
{
  pi->first_ = true;
  pi->adjust_skipto_ = false;
  pi->i_ = 0;
  pi->count_ = 0;
  pi->start_ = start;

  if (pi->start_ > max_stop())
  {
    generate_primes(pi, /* start = */ 0, /* stop = */ 0);
    pi->i_ = pi->size_ / 2;
    pi->first_ = false;
    return;
  }

  std::vector<uint64_t>& primes = to_vector(pi->primes_pimpl_);

  if (!primes.empty() &&
       primes.front() <= pi->start_ &&
       primes.back()  >= pi->start_)
  {
    pi->adjust_skipto_ = true;
    pi->i_ = std::lower_bound(primes.begin(), primes.end(), pi->start_) - primes.begin();
  }
}

/// C constructor
void primesieve_init(primesieve_iterator* pi)
{
  pi->primes_pimpl_ = reinterpret_cast<uint64_t*>(new std::vector<uint64_t>);
  primesieve_skipto(pi, 0);
}

/// C destructor
void primesieve_destroy(primesieve_iterator* pi)
{
  std::vector<uint64_t>* primes = &to_vector(pi->primes_pimpl_);
  delete primes;
}

/// Generate new next primes
void generate_next_primes(primesieve_iterator* pi)
{
  std::vector<uint64_t>& primes = to_vector(pi->primes_pimpl_);
  if (pi->adjust_skipto_)
  {
    pi->adjust_skipto_ = false;
    if (pi->i_ > 0 && primes[pi->i_ - 1] >= pi->start_)
      pi->i_--;
  }
  else
  {
    uint64_t start = (pi->first_) ? pi->start_ : primes.back() + 1;
    uint64_t interval_size = get_interval_size(pi, start);
    uint64_t stop = (start < max_stop() - interval_size) ? start + interval_size : max_stop();
    generate_primes(pi, start, stop);
    pi->i_ = 0;
  }
  pi->first_ = false;
}

/// Generate new previous primes
void generate_previous_primes(primesieve_iterator* pi)
{
  std::vector<uint64_t>& primes = to_vector(pi->primes_pimpl_);
  if (pi->adjust_skipto_)
  {
    pi->adjust_skipto_ = false;
    if (pi->i_ > 0 && primes[pi->i_] > pi->start_)
      pi->i_--;
  }
  else
  {
    uint64_t stop = pi->start_;
    if (!pi->first_)
      stop = (primes.front() > 1) ? primes.front() - 1 : 0;
    uint64_t interval_size = get_interval_size(pi, stop);
    uint64_t start = (stop > interval_size) ? stop - interval_size : 0;
    generate_primes(pi, start, stop);
    pi->i_ = primes.size();
  }
  pi->first_ = false;
}
