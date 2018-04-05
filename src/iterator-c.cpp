///
/// @file   iterator-c.cpp
/// @brief  C port of primesieve::iterator.
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/IteratorHelper.hpp>
#include <primesieve/NextPrimes.hpp>
#include <primesieve.hpp>
#include <primesieve.h>

#include <exception>
#include <cerrno>
#include <vector>

using namespace std;
using namespace primesieve;

namespace {

NextPrimes* getNextPrimes(primesieve_iterator* it)
{
  // nextPrimes_ is a pimpl
  return (NextPrimes*) it->nextPrimes_;
}

vector<uint64_t>& getPrimes(primesieve_iterator* it)
{
  // Convert pimpl to vector
  using T = vector<uint64_t>;
  T* primes = (T*) it->primes_pimpl_;
  return *primes;
}

void clearNextPrimes(primesieve_iterator* it)
{
  if (it->nextPrimes_)
    delete (NextPrimes*) it->nextPrimes_;
  it->nextPrimes_ = nullptr;
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
  it->dist_ = NextPrimes::maxCachedPrime();
  it->is_error_ = false;
  it->primes_pimpl_ = (uint64_t*) new vector<uint64_t>;
  it->nextPrimes_ = nullptr;
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
  it->dist_ = NextPrimes::maxCachedPrime();
  it->is_error_ = false;
  auto& primes = getPrimes(it);
  primes.clear();
  clearNextPrimes(it);
}

/// C destructor
void primesieve_free_iterator(primesieve_iterator* it)
{
  if (it)
  {
    clearNextPrimes(it);
    auto* primes = &getPrimes(it);
    delete primes;
  }
}

void primesieve_generate_next_primes(primesieve_iterator* it)
{
  auto& primes = getPrimes(it);
  auto nextPrimes = getNextPrimes(it);

  if (!it->is_error_)
  {
    try
    {
      if (!nextPrimes)
      {
        primes.resize(64);
        it->primes_ = &primes[0];
        IteratorHelper::next(&it->start_, &it->stop_, it->stop_hint_, &it->dist_);
        it->nextPrimes_ = (uint64_t*) new NextPrimes(it->start_, it->stop_);
        nextPrimes = (NextPrimes*) it->nextPrimes_;
      }

      for (it->last_idx_ = 0; !it->last_idx_;)
        nextPrimes->fill(&primes, &it->last_idx_);

      if (primes[0] > it->stop_)
      {
        clearNextPrimes(it);
        IteratorHelper::next(&it->start_, &it->stop_, it->stop_hint_, &it->dist_);
        it->nextPrimes_ = (uint64_t*) new NextPrimes(it->start_, it->stop_);
        nextPrimes = (NextPrimes*) it->nextPrimes_;

        for (it->last_idx_ = 0; !it->last_idx_;)
          nextPrimes->fill(&primes, &it->last_idx_);
      }

      it->i_ = 0;
      it->last_idx_--;
    }
    catch (exception&)
    {
      primes[0] = PRIMESIEVE_ERROR;
      it->i_ = 0;
      it->last_idx_ = 0;
      it->is_error_ = true;
      errno = EDOM;
    }
  }
}

void primesieve_generate_prev_primes(primesieve_iterator* it)
{
  auto& primes = getPrimes(it);

  if (!it->is_error_)
  {
    try
    {
      clearNextPrimes(it);
      primes.clear();

      while (primes.empty())
      {
        IteratorHelper::prev(&it->start_, &it->stop_, it->stop_hint_, &it->dist_);
        if (it->start_ <= 2)
          primes.push_back(0);
        generate_primes(it->start_, it->stop_, &primes);
      }
    }
    catch (exception&)
    {
      primes.clear();
      primes.push_back(PRIMESIEVE_ERROR);
      it->is_error_ = true;
      errno = EDOM;
    }
  }

  it->primes_ = &primes[0];
  it->last_idx_ = primes.size() - 1;
  it->i_ = it->last_idx_;
}
