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

void clearNextPrimes(primesieve_iterator* it)
{
  delete (NextPrimes*) it->nextPrimes_;
  it->nextPrimes_ = nullptr;
}

vector<uint64_t>& getPrimes(primesieve_iterator* it)
{
  using T = vector<uint64_t>;
  T* primes = (T*) it->primes_vector_;
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
  it->dist_ = NextPrimes::maxCachedPrime();
  it->primes_vector_ = (uint64_t*) new vector<uint64_t>;
  it->nextPrimes_ = nullptr;
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
  it->dist_ = NextPrimes::maxCachedPrime();
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

  try
  {
    while (true)
    {
      if (!it->nextPrimes_)
      {
        primes.resize(64);
        it->primes_ = &primes[0];
        IteratorHelper::next(&it->start_, &it->stop_, it->stop_hint_, &it->dist_);
        it->nextPrimes_ = (uint64_t*) new NextPrimes(it->start_, it->stop_);
        nextPrimes = getNextPrimes(it);
      }

      for (it->last_idx_ = 0; !it->last_idx_;)
        nextPrimes->fill(it->primes_, &it->last_idx_);

      if (nextPrimes->finished())
        clearNextPrimes(it);
      else
        break;
    }
  }
  catch (exception&)
  {
    clearNextPrimes(it);
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
    clearNextPrimes(it);

    while (primes.empty())
    {
      IteratorHelper::prev(&it->start_, &it->stop_, it->stop_hint_, &it->dist_);
      it->nextPrimes_ = (uint64_t*) new NextPrimes(it->start_, it->stop_);
      auto nextPrimes = getNextPrimes(it);
      if (it->start_ <= 2) primes.push_back(0);
      nextPrimes->fill(primes);
      clearNextPrimes(it);
    }
  }
  catch (exception&)
  {
    primes.clear();
    clearNextPrimes(it);
    primes.push_back(PRIMESIEVE_ERROR);
    it->is_error_ = true;
    errno = EDOM;
  }

  it->primes_ = &primes[0];
  it->last_idx_ = primes.size() - 1;
  it->i_ = it->last_idx_;
}
