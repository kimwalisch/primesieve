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
#include <primesieve/forward.hpp>
#include <primesieve/IteratorHelper.hpp>
#include <primesieve/PrimeGenerator.hpp>
#include <primesieve/resizeUninitialized.hpp>

#include <stdint.h>
#include <cerrno>
#include <exception>
#include <iostream>
#include <vector>

using namespace primesieve;

namespace {

PrimeGenerator* getPrimeGenerator(primesieve_iterator* it)
{
  // primeGenerator is a pimpl
  return (PrimeGenerator*) it->primeGenerator;
}

void clearPrimeGenerator(primesieve_iterator* it)
{
  delete getPrimeGenerator(it);
  it->primeGenerator = nullptr;
}

std::vector<uint64_t>& getPrimes(primesieve_iterator* it)
{
  using T = std::vector<uint64_t>;
  T* primes = (T*) it->vector;
  return *primes;
}

} // namespace

/// C constructor
void primesieve_init(primesieve_iterator* it)
{
  it->start = 0;
  it->stop = 0;
  it->stop_hint = get_max_stop();
  it->i = 0;
  it->last_idx = 0;
  it->dist = 0;
  it->vector = new std::vector<uint64_t>;
  it->primeGenerator = nullptr;
  it->is_error = false;
}

void primesieve_skipto(primesieve_iterator* it,
                       uint64_t start,
                       uint64_t stop_hint)
{
  it->start = start;
  it->stop = start;
  it->stop_hint = stop_hint;
  it->i = 0;
  it->last_idx = 0;
  it->dist = 0;
  auto& primes = getPrimes(it);
  primes.clear();
  clearPrimeGenerator(it);
}

/// C destructor
void primesieve_free_iterator(primesieve_iterator* it)
{
  if (it)
  {
    clearPrimeGenerator(it);
    auto* primes = &getPrimes(it);
    delete primes;
  }
}

void primesieve_generate_next_primes(primesieve_iterator* it)
{
  auto& primes = getPrimes(it);
  auto primeGenerator = getPrimeGenerator(it);
  std::size_t size = 0;

  try
  {
    while (!size)
    {
      if (!it->primeGenerator)
      {
        IteratorHelper::next(&it->start, &it->stop, it->stop_hint, &it->dist);
        it->primeGenerator = new PrimeGenerator(it->start, it->stop);
        primeGenerator = getPrimeGenerator(it);
        resizeUninitialized(primes, 512);
        it->primes = &primes[0];
      }

      primeGenerator->fillNextPrimes(primes, &size);

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
        clearPrimeGenerator(it);
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << "primesieve_iterator: " << e.what() << std::endl;
    clearPrimeGenerator(it);
    size = 1;
    primes.resize(size);
    primes[0] = PRIMESIEVE_ERROR;
    it->is_error = true;
    errno = EDOM;
  }

  it->i = 0;
  it->last_idx = size - 1;
}

void primesieve_generate_prev_primes(primesieve_iterator* it)
{
  auto& primes = getPrimes(it);
  std::size_t size = 0;

  try
  {
    if (it->primeGenerator)
      it->start = primes.front();

    primes.clear();
    clearPrimeGenerator(it);

    while (!size)
    {
      IteratorHelper::prev(&it->start, &it->stop, it->stop_hint, &it->dist);
      it->primeGenerator = new PrimeGenerator(it->start, it->stop);
      auto primeGenerator = getPrimeGenerator(it);
      if (it->start <= 2)
        primes.push_back(0);
      primeGenerator->fillPrevPrimes(primes, &size);
      clearPrimeGenerator(it);
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << "primesieve_iterator: " << e.what() << std::endl;
    clearPrimeGenerator(it);
    size = 1;
    primes.resize(size);
    primes[0] = PRIMESIEVE_ERROR;
    it->is_error = true;
    errno = EDOM;
  }

  it->primes = &primes[0];
  it->last_idx = size - 1;
  it->i = it->last_idx;
}
