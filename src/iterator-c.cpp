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

#include <stdint.h>
#include <cerrno>
#include <exception>
#include <iostream>
#include <vector>

using namespace primesieve;

namespace {

void deletePrimeGenerator(primesieve_iterator* it)
{
  delete (PrimeGenerator*) it->primeGenerator;
  it->primeGenerator = nullptr;
}

void deletePrimesVector(primesieve_iterator* it)
{
  delete (std::vector<uint64_t>*) it->vector;
  it->vector = nullptr;
}

std::vector<uint64_t>& getPrimes(void* primesPimpl)
{
  using T = std::vector<uint64_t>;
  T* primes = (T*) primesPimpl;
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
  it->primes = nullptr;
  it->vector = nullptr;
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
  it->primes = nullptr;
  deletePrimeGenerator(it);
}

/// C destructor
void primesieve_free_iterator(primesieve_iterator* it)
{
  if (it)
  {
    deletePrimeGenerator(it);
    deletePrimesVector(it);
  }
}

void primesieve_generate_next_primes(primesieve_iterator* it)
{
  std::size_t size = 0;

  try
  {
    while (!size)
    {
      auto* primeGenerator = (PrimeGenerator*) it->primeGenerator;

      if (!primeGenerator)
      {
        IteratorHelper::next(&it->start, &it->stop, it->stop_hint, &it->dist);
        primeGenerator = new PrimeGenerator(it->start, it->stop);
        it->primeGenerator = primeGenerator;
        if (!it->vector)
          it->vector = new std::vector<uint64_t>();
      }

      auto& primes = getPrimes(it->vector);

      #if defined(ENABLE_AVX512)
        if (cpuInfo.hasAVX512())
          primeGenerator->fillNextPrimesAVX512(primes, &size);
        else
      #endif
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
        deletePrimeGenerator(it);
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << "primesieve_iterator: " << e.what() << std::endl;
    deletePrimeGenerator(it);
    if (!it->vector)
      it->vector = new std::vector<uint64_t>();
    auto& primes = getPrimes(it->vector);
    size = 1;
    primes.resize(size);
    primes[0] = PRIMESIEVE_ERROR;
    it->is_error = true;
    errno = EDOM;
  }

  auto& primes = getPrimes(it->vector);
  it->i = 0;
  it->last_idx = size - 1;
  it->primes = &primes[0];
}

void primesieve_generate_prev_primes(primesieve_iterator* it)
{
  std::size_t size = 0;

  try
  {
    if (!it->vector)
      it->vector = new std::vector<uint64_t>();

    auto& primes = getPrimes(it->vector);

    // Special case if generate_next_primes() has
    // been used before generate_prev_primes().
    if (it->primeGenerator)
    {
      assert(!primes.empty());
      it->start = primes.front();
      deletePrimeGenerator(it);
    }

    while (!size)
    {
      IteratorHelper::prev(&it->start, &it->stop, it->stop_hint, &it->dist);
      PrimeGenerator primeGenerator(it->start, it->stop);
      primeGenerator.fillPrevPrimes(primes, &size);
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << "primesieve_iterator: " << e.what() << std::endl;
    deletePrimeGenerator(it);
    if (!it->vector)
      it->vector = new std::vector<uint64_t>();
    auto& primes = getPrimes(it->vector);
    size = 1;
    primes.resize(size);
    primes[0] = PRIMESIEVE_ERROR;
    it->is_error = true;
    errno = EDOM;
  }

  auto& primes = getPrimes(it->vector);
  it->last_idx = size - 1;
  it->i = it->last_idx;
  it->primes = &primes[0];
}
