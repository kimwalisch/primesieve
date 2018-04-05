///
/// @file   iterator-c.cpp
/// @brief  C port of primesieve::iterator.
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
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

void clearNextPrimes(primesieve_iterator* it)
{
  if (it->nextPrimes_)
    delete (NextPrimes*) it->nextPrimes_;

  it->nextPrimes_ = nullptr;
}

NextPrimes* getNextPrimes(primesieve_iterator* it)
{
  return (NextPrimes*) it->nextPrimes_;
}

/// Convert pimpl pointer to vector
vector<uint64_t>& to_vector(uint64_t* primes_pimpl)
{
  vector<uint64_t>* primes = reinterpret_cast<vector<uint64_t>*>(primes_pimpl);
  return *primes;
}

} // namespace

/// C constructor
void primesieve_init(primesieve_iterator* it)
{
  it->start_ = 0;
  it->stop_ = 0;
  it->stop_hint_ = primesieve_get_max_stop();
  it->i_ = 0;
  it->last_idx_ = 0;
  it->dist_ = NextPrimes::maxCachedPrime();
  it->is_error_ = false;
  it->primes_pimpl_ = (uint64_t*) new vector<uint64_t>();
  it->nextPrimes_ = nullptr;
}

/// C destructor
void primesieve_free_iterator(primesieve_iterator* it)
{
  if (it)
  {
    clearNextPrimes(it);
    vector<uint64_t>* primes = &to_vector(it->primes_pimpl_);
    delete primes;
  }
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

  vector<uint64_t>& primes = to_vector(it->primes_pimpl_);
  primes.clear();
  clearNextPrimes(it);
}

void primesieve_generate_next_primes(primesieve_iterator* it)
{
  vector<uint64_t>& primes = to_vector(it->primes_pimpl_);
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
      it->i_ = 1;
      it->last_idx_ = 1;
      it->is_error_ = true;
      errno = EDOM;
    }
  }
}

void primesieve_generate_prev_primes(primesieve_iterator* it)
{
  vector<uint64_t>& primes = to_vector(it->primes_pimpl_);

  if (!it->is_error_)
  {
    try
    {
      primes.clear();
      clearNextPrimes(it);

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
      primes.resize(64, PRIMESIEVE_ERROR);
      it->is_error_ = true;
      errno = EDOM;
    }
  }

  it->primes_ = &primes[0];
  it->last_idx_ = primes.size() - 1;
  it->i_ = it->last_idx_;
}
