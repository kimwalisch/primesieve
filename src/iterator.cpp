///
/// @file  iterator.cpp
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/iterator.hpp>
#include <primesieve/IteratorHelper.hpp>
#include <primesieve/PrimeGenerator.hpp>

#include <stdint.h>
#include <vector>
#include <memory>

namespace {

template <typename T>
void clear(std::unique_ptr<T>& ptr)
{
  ptr.reset(nullptr);
}

} // namespace

namespace primesieve {

iterator::~iterator() = default;

iterator::iterator(iterator&&) noexcept = default;

iterator& iterator::operator=(iterator&&) noexcept = default;

iterator::iterator(uint64_t start,
                   uint64_t stop_hint)
{
  skipto(start, stop_hint);
}

void iterator::skipto(uint64_t start,
                      uint64_t stop_hint)
{
  start_ = start;
  stop_ = start;
  stop_hint_ = stop_hint;
  i_ = 0;
  last_idx_ = 0;
  dist_ = 0;
  clear(primeGenerator_);
}

void iterator::generate_next_primes()
{
  std::size_t size = 0;

  while (!size)
  {
    if (!primeGenerator_)
    {
      IteratorHelper::next(&start_, &stop_, stop_hint_, &dist_);
      auto p = new PrimeGenerator(start_, stop_);
      primeGenerator_.reset(p);
    }

    primeGenerator_->fillNextPrimes(primes_, &size);

    // There are 3 different cases here:
    // 1) The primes array contains a few primes (<= 512).
    //    In this case we return the primes to the user.
    // 2) The primes array is empty because the next
    //    prime > stop. In this case we reset the
    //    primeGenerator object, increase the start & stop
    //    numbers and sieve the next segment.
    // 3) The next prime > 2^64. In this case the primes
    //    array contains an error code (UINT64_MAX) which
    //    is returned to the user.
    if (size == 0)
      clear(primeGenerator_);
  }

  i_ = 0;
  last_idx_ = size - 1;
}

void iterator::generate_prev_primes()
{
  if (primeGenerator_)
    start_ = primes_.front();

  std::size_t size = 0;

  while (!size)
  {
    IteratorHelper::prev(&start_, &stop_, stop_hint_, &dist_);
    auto p = new PrimeGenerator(start_, stop_);
    primeGenerator_.reset(p);
    primeGenerator_->fillPrevPrimes(primes_, &size);
    clear(primeGenerator_);
  }

  last_idx_ = size - 1;
  i_ = last_idx_;
}

} // namespace
