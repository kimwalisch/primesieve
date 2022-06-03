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
#include <limits>
#include <iostream>

namespace {

using namespace primesieve;

pod_vector<uint64_t>& getPrimes(primesieve_iterator* it)
{
  auto* memory = (IteratorMemory*) it->memory;
  return memory->primes;
}

} // namespace

/// C constructor
void primesieve_init(primesieve_iterator* it)
{
  it->i = 0;
  it->last_idx = 0;
  it->start = 0;
  it->stop_hint = std::numeric_limits<uint64_t>::max();
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
  it->stop_hint = stop_hint;
  it->primes = nullptr;

  if (it->memory)
  {
    auto* memory = (IteratorMemory*) it->memory;
    memory->stop = start;
    memory->dist = 0;
    primesieve_clear(it);
  }
}

/// C destructor
void primesieve_free_iterator(primesieve_iterator* it)
{
  if (it && it->memory)
  {
    delete (IteratorMemory*) it->memory;
    it->memory = nullptr;
  }
}

/// Frees most memory, but keeps some smaller data structures
/// (e.g. the PreSieve object) that are useful if the
/// primesieve_iterator is reused. The remaining memory
/// uses at most 200 kilobytes.
///
void primesieve_clear(primesieve_iterator* it)
{
  if (it->memory)
  {
    auto* memory = (IteratorMemory*) it->memory;
    memory->deletePrimeGenerator();
    memory->deletePrimes();
  }
}

void primesieve_generate_next_primes(primesieve_iterator* it)
{
  std::size_t size = 0;

  try
  {
    if (!it->memory)
      it->memory = new IteratorMemory(it->start);

    auto& memory = *(IteratorMemory*) it->memory;
    auto& primes = memory.primes;

    while (!size)
    {
      if (!memory.primeGenerator)
      {
        IteratorHelper::next(&it->start, &memory.stop, it->stop_hint, &memory.dist);
        memory.primeGenerator = new PrimeGenerator(it->start, memory.stop, memory.preSieve);
      }

      memory.primeGenerator->fillNextPrimes(primes, &size);

      // There are 3 different cases here:
      // 1) The primes array contains a few primes (<= 1024).
      //    In this case we return the primes to the user.
      // 2) The primes array is empty because the next
      //    prime > stop. In this case we reset the
      //    primeGenerator object, increase the start & stop
      //    numbers and sieve the next segment.
      // 3) The next prime > 2^64. In this case the primes
      //    array contains an error code (UINT64_MAX) which
      //    is returned to the user.
      if (size == 0)
        memory.deletePrimeGenerator();
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << "primesieve_iterator: " << e.what() << std::endl;
    primesieve_clear(it);
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
      it->memory = new IteratorMemory(it->start);

    auto& memory = *(IteratorMemory*) it->memory;
    auto& primes = memory.primes;

    // Special case if generate_next_primes() has
    // been used before generate_prev_primes().
    if_unlikely(memory.primeGenerator)
    {
      it->start = primes.front();
      memory.deletePrimeGenerator();
    }

    // When sieving backwards the sieving distance is subdivided
    // into smaller chunks. If we can prove that the total
    // sieving distance is large we enable pre-sieving.
    if (memory.dist == 0 &&
        it->stop_hint < it->start)
      memory.preSieve.init(it->stop_hint, it->start);

    while (!size)
    {
      IteratorHelper::prev(&it->start, &memory.stop, it->stop_hint, &memory.dist);
      PrimeGenerator primeGenerator(it->start, memory.stop, memory.preSieve);
      primeGenerator.fillPrevPrimes(primes, &size);
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << "primesieve_iterator: " << e.what() << std::endl;
    primesieve_clear(it);
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
