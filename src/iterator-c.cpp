///
/// @file   iterator-c.cpp
/// @brief  C port of primesieve::iterator.
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve.h>
#include <primesieve/IteratorHelper.hpp>
#include <primesieve/PrimeGenerator.hpp>
#include <primesieve/macros.hpp>
#include <primesieve/pod_vector.hpp>

#include <stdint.h>
#include <cerrno>
#include <exception>
#include <iostream>

namespace {

using namespace primesieve;

pod_vector<uint64_t>& getPrimes(primesieve_iterator* it)
{
  auto* memory = (IteratorMemory*) it->memory;
  return memory->primes;
}

/// Frees all memory except the IteratorMemory struct (it->memory)
void freeMostMemory(primesieve_iterator* it)
{
  if (it->memory)
  {
    auto* memory = (IteratorMemory*) it->memory;
    delete memory->primeGenerator;
    memory->primeGenerator = nullptr;
    memory->memoryPool.clear();
  }
}

} // namespace

/// C constructor
void primesieve_init(primesieve_iterator* it)
{
  it->i = 0;
  it->last_idx = 0;
  it->start = 0;
  it->stop = 0;
  it->stop_hint = std::numeric_limits<uint64_t>::max();
  it->dist = 0;
  it->primes = nullptr;
  it->memory = nullptr;
  it->is_error = false;
}

void primesieve_skipto(primesieve_iterator* it,
                       uint64_t start,
                       uint64_t stop_hint)
{
  it->i = 0;
  it->last_idx = 0;
  it->start = start;
  it->stop = start;
  it->stop_hint = stop_hint;
  it->dist = 0;
  it->primes = nullptr;
  freeMostMemory(it);
}

/// C destructor
void primesieve_free_iterator(primesieve_iterator* it)
{
  if (it && it->memory)
  {
    auto* memory = (IteratorMemory*) it->memory;
    delete memory->primeGenerator;
    delete memory;
    it->memory = nullptr;
  }
}

void primesieve_generate_next_primes(primesieve_iterator* it)
{
  std::size_t size = 0;

  try
  {
    if (!it->memory)
      it->memory = new IteratorMemory;

    auto& memory = *(IteratorMemory*) it->memory;
    auto& primes = memory.primes;

    while (!size)
    {
      if (!memory.primeGenerator)
      {
        IteratorHelper::next(&it->start, &it->stop, it->stop_hint, &it->dist);
        memory.primeGenerator = new PrimeGenerator(it->start, it->stop, memory.preSieve, memory.memoryPool);
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
        freeMostMemory(it);
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << "primesieve_iterator: " << e.what() << std::endl;
    freeMostMemory(it);
    auto& primes = getPrimes(it);
    primes.clear();
    primes.push_back(PRIMESIEVE_ERROR);
    size = primes.size();
    it->is_error = true;
    errno = EDOM;
  }

  auto& primes = getPrimes(it);
  it->i = 0;
  it->last_idx = size - 1;
  it->primes = &primes[0];
}

void primesieve_generate_prev_primes(primesieve_iterator* it)
{
  std::size_t size = 0;

  try
  {
    if (!it->memory)
      it->memory = new IteratorMemory;

    auto& memory = *(IteratorMemory*) it->memory;
    auto& primes = memory.primes;

    // Special case if generate_next_primes() has
    // been used before generate_prev_primes().
    if_unlikely(memory.primeGenerator)
    {
      assert(!primes.empty());
      it->start = primes.front();
      freeMostMemory(it);
    }

    while (!size)
    {
      IteratorHelper::prev(&it->start, &it->stop, it->stop_hint, &it->dist);
      PrimeGenerator primeGenerator(it->start, it->stop, memory.preSieve, memory.memoryPool);
      primeGenerator.fillPrevPrimes(primes, &size);
      memory.memoryPool.clear();
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << "primesieve_iterator: " << e.what() << std::endl;
    freeMostMemory(it);
    auto& primes = getPrimes(it);
    primes.clear();
    primes.push_back(PRIMESIEVE_ERROR);
    size = primes.size();
    it->is_error = true;
    errno = EDOM;
  }

  auto& primes = getPrimes(it);
  it->last_idx = size - 1;
  it->i = it->last_idx;
  it->primes = &primes[0];
}
