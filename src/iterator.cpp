///
/// @file  iterator.cpp
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/iterator.hpp>
#include <primesieve/IteratorHelper.hpp>
#include <primesieve/PrimeGenerator.hpp>

#include <stdint.h>
#include <vector>

using namespace primesieve;

namespace {

void clear(PrimeGenerator*& ptr)
{
  delete ptr;
  ptr = nullptr;
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
  dist_ = PrimeGenerator::maxCachedPrime();
  primeGenerator_ = nullptr;
}

void iterator::skipto(uint64_t start, uint64_t stop_hint)
{
  start_ = start;
  stop_ = start;
  stop_hint_ = stop_hint;
  i_ = 0;
  last_idx_ = 0;
  dist_ = PrimeGenerator::maxCachedPrime();
  clear(primeGenerator_);
  primes_.clear();
}

iterator::~iterator()
{
  clear(primeGenerator_);
}

void iterator::generate_next_primes()
{
  while (true)
  {
    if (!primeGenerator_)
    {
      primes_.resize(64);
      IteratorHelper::next(&start_, &stop_, stop_hint_, &dist_);
      primeGenerator_ = new PrimeGenerator(start_, stop_);
    }

    for (last_idx_ = 0; !last_idx_;)
      primeGenerator_->fill(primes_, &last_idx_);

    if (primeGenerator_->finished())
      clear(primeGenerator_);
    else
      break;
  }

  i_ = 0;
  last_idx_--;
}

void iterator::generate_prev_primes()
{
  primes_.clear();
  clear(primeGenerator_);

  while (primes_.empty())
  {
    IteratorHelper::prev(&start_, &stop_, stop_hint_, &dist_);
    if (start_ <= 2)
      primes_.push_back(0);
    primeGenerator_ = new PrimeGenerator(start_, stop_);
    primeGenerator_->fill(primes_);
    clear(primeGenerator_);
  }

  last_idx_ = primes_.size() - 1;
  i_ = last_idx_;
}

} // namespace
