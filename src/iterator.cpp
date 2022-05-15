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

namespace {

void deletePrimeGenerator(primesieve::iterator* it)
{
  if (it->memory_)
  {
    using primesieve::IteratorMemory;
    auto* memory = (IteratorMemory*) it->memory_;
    delete memory->primeGenerator;
    memory->primeGenerator = nullptr;
  }
}

} // namespace

namespace primesieve {

iterator::iterator() noexcept :
  i_(0),
  last_idx_(0),
  start_(0),
  stop_hint_(std::numeric_limits<uint64_t>::max()),
  primes_(nullptr),
  memory_(nullptr)
{ }

iterator::iterator(uint64_t start,
                   uint64_t stop_hint) noexcept :
  i_(0),
  last_idx_(0),
  start_(start),
  stop_hint_(stop_hint),
  primes_(nullptr),
  memory_(nullptr)
{ }

/// Move constructor
iterator::iterator(iterator&& other) noexcept :
  i_(other.i_),
  last_idx_(other.last_idx_),
  start_(other.start_),
  stop_hint_(other.stop_hint_),
  primes_(other.primes_),
  memory_(other.memory_)
{
  other.i_ = 0;
  other.last_idx_ = 0;
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
    clear();

    i_ = other.i_;
    last_idx_ = other.last_idx_;
    start_ = other.start_;
    stop_hint_ = other.stop_hint_;
    primes_ = other.primes_;
    memory_ = other.memory_;

    other.i_ = 0;
    other.last_idx_ = 0;
    other.start_ = 0;
    other.stop_hint_ = std::numeric_limits<uint64_t>::max();
    other.primes_ = nullptr;
    other.memory_ = nullptr;
  }

  return *this;
}

void iterator::skipto(uint64_t start,
                      uint64_t stop_hint) noexcept
{
  i_ = 0;
  last_idx_ = 0;
  start_ = start;
  stop_hint_ = stop_hint;
  primes_ = nullptr;

  if (memory_)
  {
    auto* memory = (IteratorMemory*) memory_;
    memory->stop = start;
    memory->dist = 0;
    deletePrimeGenerator(this);
  }
}

iterator::~iterator()
{
  clear();
}

void iterator::clear() noexcept
{
  if (memory_)
  {
    auto* memory = (IteratorMemory*) memory_;
    delete memory->primeGenerator;
    delete memory;
    memory_ = nullptr;
  }
}

void iterator::generate_next_primes()
{
  if (!memory_)
    memory_ = new IteratorMemory(start_);

  auto& memory = *(IteratorMemory*) memory_;
  auto& primes = memory.primes;
  std::size_t size = 0;

  while (!size)
  {
    if (!memory.primeGenerator)
    {
      IteratorHelper::next(&start_, &memory.stop, stop_hint_, &memory.dist);
      memory.primeGenerator = new PrimeGenerator(start_, memory.stop, memory.preSieve);
    }

    memory.primeGenerator->fillNextPrimes(primes, &size);

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
      deletePrimeGenerator(this);
  }

  i_ = 0;
  last_idx_ = size - 1;
  primes_ = &primes[0];
}

void iterator::generate_prev_primes()
{
  if (!memory_)
    memory_ = new IteratorMemory(start_);

  auto& memory = *(IteratorMemory*) memory_;
  auto& primes = memory.primes;

  // Special case if generate_next_primes() has
  // been used before generate_prev_primes().
  if_unlikely(memory.primeGenerator)
  {
    assert(!primes.empty());
    start_ = primes.front();
    deletePrimeGenerator(this);
  }

  std::size_t size = 0;

  while (!size)
  {
    IteratorHelper::prev(&start_, &memory.stop, stop_hint_, &memory.dist);
    PrimeGenerator primeGenerator(start_, memory.stop, memory.preSieve);
    primeGenerator.fillPrevPrimes(primes, &size);
  }

  last_idx_ = size - 1;
  i_ = last_idx_;
  primes_ = &primes[0];
}

} // namespace
