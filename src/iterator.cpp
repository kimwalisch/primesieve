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

void freeAllMemory(primesieve::iterator* it)
{
  if (it->memory_)
  {
    using primesieve::IteratorMemory;
    auto* memory = (IteratorMemory*) it->memory_;
    delete memory->primeGenerator;
    delete memory;
    it->memory_ = nullptr;
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
    freeAllMemory(this);

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
    clear();
  }
}

iterator::~iterator()
{
  freeAllMemory(this);
}

/// Frees most memory, but keeps some smaller data structures
/// (e.g. primes vector & PreSieve object) that are useful
/// if the primesieve::iterator is reused. The remaining memory
/// uses at most 200 kilobytes.
///
void iterator::clear() noexcept
{
  if (memory_)
  {
    auto* memory = (IteratorMemory*) memory_;
    delete memory->primeGenerator;
    memory->primeGenerator = nullptr;

    // Delete the primes vector if > 100 KiB.
    // next_prime() uses primes vector of 4 KiB, but
    // prev_prime() uses primes vector of up to 1 GiB.
    std::size_t maxSize = ((1 << 10) * 100) / sizeof(uint64_t);
    if (memory->primes.size() > maxSize)
      pod_vector<uint64_t>().swap(memory->primes);
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
      clear();
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
    clear();
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
