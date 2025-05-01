///
/// @file  iterator.cpp
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include "IteratorHelper.hpp"
#include "PrimeGenerator.hpp"

#include <primesieve/iterator.hpp>
#include <primesieve/macros.hpp>

#include <stdint.h>
#include <limits>

namespace {

void freeAllMemory(primesieve::iterator* it)
{
  if (it->memory_)
  {
    using primesieve::IteratorData;
    delete (IteratorData*) it->memory_;
    it->memory_ = nullptr;
  }
}

} // namespace

namespace primesieve {

iterator::iterator() noexcept :
  iterator(0)
{ }

iterator::iterator(uint64_t start,
                   uint64_t stop_hint) noexcept :
  i_(0),
  size_(0),
  start_(start),
  stop_hint_(stop_hint),
  primes_(nullptr),
  memory_(nullptr)
{ }

/// Move constructor
iterator::iterator(iterator&& other) noexcept :
  i_(other.i_),
  size_(other.size_),
  start_(other.start_),
  stop_hint_(other.stop_hint_),
  primes_(other.primes_),
  memory_(other.memory_)
{
  other.i_ = 0;
  other.size_ = 0;
  other.start_ = 0;
  other.stop_hint_ = std::numeric_limits<uint64_t>::max();
  other.primes_ = nullptr;
  other.memory_ = nullptr;
}

/// Move assignment operator
iterator& iterator::operator=(iterator&& other) noexcept
{
  if (this != &other)
  {
    freeAllMemory(this);

    i_ = other.i_;
    size_ = other.size_;
    start_ = other.start_;
    stop_hint_ = other.stop_hint_;
    primes_ = other.primes_;
    memory_ = other.memory_;

    other.i_ = 0;
    other.size_ = 0;
    other.start_ = 0;
    other.stop_hint_ = std::numeric_limits<uint64_t>::max();
    other.primes_ = nullptr;
    other.memory_ = nullptr;
  }

  return *this;
}

void iterator::jump_to(uint64_t start,
                       uint64_t stop_hint) noexcept
{
  i_ = 0;
  size_ = 0;
  start_ = start;
  stop_hint_ = stop_hint;
  primes_ = nullptr;

  // Frees most memory, but keeps some smaller data
  // structures (e.g. the IteratorData object) that
  // are useful if the primesieve::iterator is reused.
  // The remaining memory uses at most 2 kilobytes.
  if (memory_)
  {
    auto& iterData = *(IteratorData*) memory_;
    iterData.stop = start;
    iterData.dist = 0;
    iterData.include_start_number = true;
    iterData.deletePrimeGenerator();
    iterData.deletePrimes();
  }
}

void iterator::clear() noexcept
{
  jump_to(0);
}

iterator::~iterator()
{
  freeAllMemory(this);
}

void iterator::generate_next_primes()
{
  if (!memory_)
    memory_ = new IteratorData(start_);

  auto& iterData = *(IteratorData*) memory_;
  auto& primes = iterData.primes;

  while (true)
  {
    if (!iterData.primeGenerator)
    {
      IteratorHelper::updateNext(start_, stop_hint_, iterData);
      iterData.newPrimeGenerator(start_, iterData.stop);
    }

    iterData.primeGenerator->fillNextPrimes(primes, &size_);
    primes_ = primes.data();
    i_ = 0;

    // There are 2 different cases here:
    // 1) The primes array is empty because the next prime > stop.
    //    In this case we reset the primeGenerator object, increase
    //    the start & stop numbers and sieve the next segment.
    // 2) The primes array is not empty (contains up to 1024 primes),
    //    in this case we return it to the user.
    if_unlikely(size_ == 0)
      iterData.deletePrimeGenerator();
    else
      return;
  }
}

void iterator::generate_prev_primes()
{
  if (!memory_)
    memory_ = new IteratorData(start_);

  auto& iterData = *(IteratorData*) memory_;
  auto& primes = iterData.primes;

  // Special case if generate_next_primes() has
  // been used before generate_prev_primes().
  if_unlikely(iterData.primeGenerator)
  {
    start_ = primes.front();
    iterData.deletePrimeGenerator();
    ASSERT(!iterData.include_start_number);
  }

  do
  {
    IteratorHelper::updatePrev(start_, stop_hint_, iterData);
    iterData.newPrimeGenerator(start_, iterData.stop);
    iterData.primeGenerator->fillPrevPrimes(primes, &size_);
    iterData.deletePrimeGenerator();
    primes_ = primes.data();
    i_ = size_;
  }
  while (!size_);
}

} // namespace
