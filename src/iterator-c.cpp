///
/// @file   iterator-c.cpp
/// @brief  C port of primesieve::iterator.
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include "IteratorHelper.hpp"
#include "PrimeGenerator.hpp"

#include <primesieve.h>
#include <primesieve/macros.hpp>
#include <primesieve/Vector.hpp>

#include <stdint.h>
#include <cerrno>
#include <exception>
#include <limits>
#include <iostream>

namespace {

using namespace primesieve;

IteratorData& getIterData(primesieve_iterator* it)
{
  ASSERT(it->memory != nullptr);
  return *(IteratorData*) it->memory;
}

Vector<uint64_t>& getPrimes(primesieve_iterator* it)
{
  return getIterData(it).primes;
}

} // namespace

/// C constructor
void primesieve_init(primesieve_iterator* it)
{
  it->i = 0;
  it->size = 0;
  it->start = 0;
  it->stop_hint = std::numeric_limits<uint64_t>::max();
  it->primes = nullptr;
  it->memory = nullptr;
  it->is_error = false;
}

void primesieve_jump_to(primesieve_iterator* it,
                        uint64_t start,
                        uint64_t stop_hint)
{
  it->i = 0;
  it->size = 0;
  it->start = start;
  it->stop_hint = stop_hint;
  it->primes = nullptr;

  // Frees most memory, but keeps some smaller data
  // structures (e.g. the IteratorData object) that
  // are useful if the primesieve_iterator is reused.
  // The remaining memory uses at most 2 kilobytes.
  if (it->memory)
  {
    auto& iterData = getIterData(it);
    iterData.stop = start;
    iterData.dist = 0;
    iterData.include_start_number = true;
    iterData.deletePrimeGenerator();
    iterData.deletePrimes();
  }
}

/// Deprecated, use primesieve_jump_to() instead.
/// primesieve_jump_to() includes the start number,
/// whereas primesieve_skipto() excludes the start number.
///
void primesieve_skipto(primesieve_iterator* it,
                       uint64_t start,
                       uint64_t stop_hint)
{
  it->i = 0;
  it->size = 0;
  it->start = start;
  it->stop_hint = stop_hint;
  it->primes = nullptr;

  if (!it->memory)
    it->memory = new IteratorData(it->start);

  auto& iterData = getIterData(it);
  iterData.stop = start;
  iterData.dist = 0;
  iterData.include_start_number = false;
  iterData.deletePrimeGenerator();
  iterData.deletePrimes();
}

void primesieve_clear(primesieve_iterator* it)
{
  primesieve_jump_to(it, 0, std::numeric_limits<uint64_t>::max());
}

/// C destructor
void primesieve_free_iterator(primesieve_iterator* it)
{
  if (it && it->memory)
  {
    delete (IteratorData*) it->memory;
    it->memory = nullptr;
  }
}

void primesieve_generate_next_primes(primesieve_iterator* it)
{
  try
  {
    if (!it->memory)
      it->memory = new IteratorData(it->start);

    auto& iterData = getIterData(it);
    auto& primes = iterData.primes;

    while (true)
    {
      if (!iterData.primeGenerator)
      {
        IteratorHelper::updateNext(it->start, it->stop_hint, iterData);
        iterData.newPrimeGenerator(it->start, iterData.stop);
      }

      iterData.primeGenerator->fillNextPrimes(primes, &it->size);
      it->primes = primes.data();
      it->i = 0;

      // There are 2 different cases here:
      // 1) The primes array is empty because the next prime > stop.
      //    In this case we reset the primeGenerator object, increase
      //    the start & stop numbers and sieve the next segment.
      // 2) The primes array is not empty (contains up to 1024 primes),
      //    in this case we return it to the user.
      if_unlikely(it->size == 0)
        iterData.deletePrimeGenerator();
      else
        return;
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << "primesieve_iterator: " << e.what() << std::endl;
    primesieve_clear(it);
    auto& primes = getPrimes(it);
    ASSERT(primes.empty());
    primes.push_back(PRIMESIEVE_ERROR);
    getIterData(it).stop = PRIMESIEVE_ERROR;
    it->primes = primes.data();
    it->size = primes.size();
    it->i = 0;
    it->is_error = true;
    errno = EDOM;
  }
}

void primesieve_generate_prev_primes(primesieve_iterator* it)
{
  try
  {
    if (!it->memory)
      it->memory = new IteratorData(it->start);

    auto& iterData = getIterData(it);
    auto& primes = iterData.primes;

    // Special case if generate_next_primes() has
    // been used before generate_prev_primes().
    if_unlikely(iterData.primeGenerator)
    {
      it->start = primes.front();
      iterData.deletePrimeGenerator();
      ASSERT(!iterData.include_start_number);
    }

    do
    {
      IteratorHelper::updatePrev(it->start, it->stop_hint, iterData);
      iterData.newPrimeGenerator(it->start, iterData.stop);
      iterData.primeGenerator->fillPrevPrimes(primes, &it->size);
      iterData.deletePrimeGenerator();
      it->primes = primes.data();
      it->i = it->size;
    }
    while (!it->size);
  }
  catch (const std::exception& e)
  {
    std::cerr << "primesieve_iterator: " << e.what() << std::endl;
    primesieve_clear(it);
    auto& primes = getPrimes(it);
    ASSERT(primes.empty());
    primes.push_back(PRIMESIEVE_ERROR);
    it->primes = primes.data();
    it->size = primes.size();
    it->i = it->size;
    it->is_error = true;
    errno = EDOM;
  }
}
