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
#include <primesieve/macros.hpp>
#include <primesieve/pod_vector.hpp>

#include <stdint.h>
#include <cassert>
#include <cstring>

namespace {

using namespace primesieve;

void deletePrimeGenerator(primesieve::iterator* it)
{
  delete (PrimeGenerator*) it->primeGenerator_;
  it->primeGenerator_ = nullptr;
}

void deletePrimesVector(primesieve::iterator* it)
{
  delete (pod_vector<uint64_t>*) it->primesVector_;
  it->primesVector_ = nullptr;
}

pod_vector<uint64_t>& getPrimes(primesieve::iterator* it)
{
  return *(pod_vector<uint64_t>*) it->primesVector_;
}

} // namespace

namespace primesieve {

iterator::iterator() noexcept
{
  i_ = 0;
  last_idx_ = 0;
  start_ = 0;
  stop_ = 0;
  stop_hint_ = std::numeric_limits<uint64_t>::max();
  dist_ = 0;
  primes_ = nullptr;
  primesVector_ = nullptr;
  primeGenerator_ = nullptr;
}

iterator::iterator(uint64_t start,
                   uint64_t stop_hint) noexcept
{
  i_ = 0;
  last_idx_ = 0;
  start_ = start;
  stop_ = start;
  stop_hint_ = stop_hint;
  dist_ = 0;
  primes_ = nullptr;
  primesVector_ = nullptr;
  primeGenerator_ = nullptr;
}

/// Move constructor
iterator::iterator(iterator&& other) noexcept
{
  i_ = other.i_;
  last_idx_ = other.last_idx_;
  start_ = other.start_;
  stop_ = other.stop_;
  stop_hint_ = other.stop_hint_;
  dist_ = other.dist_;
  primes_ = other.primes_;
  primesVector_ = other.primesVector_;
  primeGenerator_ = other.primeGenerator_;

  other.i_ = 0;
  other.last_idx_ = 0;
  other.start_ = 0;
  other.stop_ = 0;
  other.stop_hint_ = std::numeric_limits<uint64_t>::max();
  other.dist_ = 0;
  other.primes_ = nullptr;
  other.primesVector_ = nullptr;
  other.primeGenerator_ = nullptr;
}

/// Move assignment operator
iterator& iterator::operator=(iterator&& other) noexcept
{
  if (this != &other)
  {
    i_ = other.i_;
    last_idx_ = other.last_idx_;
    start_ = other.start_;
    stop_ = other.stop_;
    stop_hint_ = other.stop_hint_;
    dist_ = other.dist_;
    primes_ = other.primes_;
    primesVector_ = other.primesVector_;
    primeGenerator_ = other.primeGenerator_;

    other.i_ = 0;
    other.last_idx_ = 0;
    other.start_ = 0;
    other.stop_ = 0;
    other.stop_hint_ = std::numeric_limits<uint64_t>::max();
    other.dist_ = 0;
    other.primes_ = nullptr;
    other.primesVector_ = nullptr;
    other.primeGenerator_ = nullptr;
  }

  return *this;
}

void iterator::skipto(uint64_t start,
                      uint64_t stop_hint)
{
  i_ = 0;
  last_idx_ = 0;
  start_ = start;
  stop_ = start;
  stop_hint_ = stop_hint;
  dist_ = 0;
  primes_ = nullptr;
  deletePrimeGenerator(this);
}

iterator::~iterator()
{
  clear();
}

void iterator::clear()
{
  deletePrimeGenerator(this);
  deletePrimesVector(this);
}

void iterator::generate_next_primes()
{
  std::size_t size = 0;

  while (!size)
  {
    auto* primeGenerator = (PrimeGenerator*) primeGenerator_;

    if (!primeGenerator)
    {
      deletePrimeGenerator(this);
      IteratorHelper::next(&start_, &stop_, stop_hint_, &dist_);
      primeGenerator = new PrimeGenerator(start_, stop_);
      primeGenerator_ = primeGenerator;
      if (!primesVector_)
        primesVector_ = new pod_vector<uint64_t>();
    }

    auto& primes = getPrimes(this);
    primeGenerator->fillNextPrimes(primes, &size);

    // There are 3 different cases here:
    // 1) The primes array contains a few primes (<= 512).
    //    In this case we return the primes to the user.
    // 2) The primes array is empty because the next
    //    prime > stop_. In this case we reset the
    //    primeGenerator object, increase the start_ & stop_
    //    numbers and sieve the next segment.
    // 3) The next prime > 2^64. In this case the primes
    //    array contains an error code (UINT64_MAX) which
    //    is returned to the user.
    if (size == 0)
      deletePrimeGenerator(this);
  }

  auto& primes = getPrimes(this);
  i_ = 0;
  last_idx_ = size - 1;
  primes_ = &primes[0];
}

void iterator::generate_prev_primes()
{
  if (!primesVector_)
    primesVector_ = new pod_vector<uint64_t>();

  auto& primes = getPrimes(this);

  // Special case if generate_next_primes() has
  // been used before generate_prev_primes().
  if_unlikely(primeGenerator_)
  {
    assert(!primes.empty());
    start_ = primes.front();
    deletePrimeGenerator(this);
  }

  std::size_t size = 0;

  while (!size)
  {
    IteratorHelper::prev(&start_, &stop_, stop_hint_, &dist_);
    PrimeGenerator primeGenerator(start_, stop_);
    primeGenerator.fillPrevPrimes(primes, &size);
  }

  last_idx_ = size - 1;
  i_ = last_idx_;
  primes_ = &primes[0];
}

} // namespace
