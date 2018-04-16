///
/// @file   iterator-c.cpp
/// @brief  C port of primesieve::iterator.
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve.h>
#include <primesieve.hpp>
#include <primesieve/IteratorHelper.hpp>
#include <primesieve/PrimeGenerator.hpp>

#include <stdint.h>
#include <cerrno>
#include <exception>
#include <vector>

using namespace std;
using namespace primesieve;

namespace {

PrimeGenerator* getPrimeGenerator(primesieve_iterator* it)
{
  // primeGenerator_ is a pimpl
  return (PrimeGenerator*) it->primeGenerator_;
}

void clearPrimeGenerator(primesieve_iterator* it)
{
  delete getPrimeGenerator(it);
  it->primeGenerator_ = nullptr;
}

vector<uint64_t>& getPrimes(primesieve_iterator* it)
{
  using T = vector<uint64_t>;
  T* primes = (T*) it->vector_;
  return *primes;
}

} // namespace

/// C constructor
void primesieve_init(primesieve_iterator* it)
{
  it->start_ = 0;
  it->stop_ = 0;
  it->stop_hint_ = get_max_stop();
  it->i_ = 0;
  it->last_idx_ = 0;
  it->dist_ = PrimeGenerator::maxCachedPrime();
  it->vector_ = new vector<uint64_t>;
  it->primeGenerator_ = nullptr;
  it->is_error_ = false;
}

void primesieve_skipto(primesieve_iterator* it,
                       uint64_t start,
                       uint64_t stop_hint)
{
  it->start_ = start;
  it->stop_ = start;
  it->stop_hint_ = stop_hint;
  it->i_ = 0;
  it->last_idx_ = 0;
  it->dist_ = PrimeGenerator::maxCachedPrime();
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

  try
  {
    while (true)
    {
      if (!it->primeGenerator_)
      {
        IteratorHelper::next(&it->start_, &it->stop_, it->stop_hint_, &it->dist_);
        it->primeGenerator_ = new PrimeGenerator(it->start_, it->stop_);
        primeGenerator = getPrimeGenerator(it);
        primes.resize(64);
        it->primes_ = &primes[0];
      }

      for (it->last_idx_ = 0; !it->last_idx_;)
        primeGenerator->fill(primes, &it->last_idx_);

      if (primeGenerator->finished())
        clearPrimeGenerator(it);
      else
        break;
    }
  }
  catch (exception&)
  {
    clearPrimeGenerator(it);
    primes.resize(1);
    primes[0] = PRIMESIEVE_ERROR;
    it->last_idx_ = 1;
    it->is_error_ = true;
    errno = EDOM;
  }

  it->i_ = 0;
  it->last_idx_--;
}

void primesieve_generate_prev_primes(primesieve_iterator* it)
{
  auto& primes = getPrimes(it);

  try
  {
    primes.clear();
    clearPrimeGenerator(it);

    while (primes.empty())
    {
      IteratorHelper::prev(&it->start_, &it->stop_, it->stop_hint_, &it->dist_);
      it->primeGenerator_ = new PrimeGenerator(it->start_, it->stop_);
      auto primeGenerator = getPrimeGenerator(it);
      if (it->start_ <= 2)
        primes.push_back(0);
      primeGenerator->fill(primes);
      clearPrimeGenerator(it);
    }
  }
  catch (exception&)
  {
    clearPrimeGenerator(it);
    primes.resize(1);
    primes[0] = PRIMESIEVE_ERROR;
    it->is_error_ = true;
    errno = EDOM;
  }

  it->primes_ = &primes[0];
  it->last_idx_ = primes.size() - 1;
  it->i_ = it->last_idx_;
}
