///
/// @file  IteratorHelper.cpp
///        Functions used to calculate the next start and stop
///        numbers for primesieve::iterator.
///
/// Copyright (C) 2023 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include "IteratorHelper.hpp"
#include "PrimeGenerator.hpp"

#include <primesieve/config.hpp>
#include <primesieve/pmath.hpp>

#include <stdint.h>
#include <algorithm>
#include <cmath>
#include <limits>

using namespace primesieve;

namespace {

uint64_t getNextDist(uint64_t start, uint64_t dist)
{
  uint64_t minDist = (uint64_t) std::sqrt(start);
  uint64_t maxDist = 1ull << 60;

  dist *= 4;
  minDist = std::max(minDist, PrimeGenerator::maxCachedPrime());
  return inBetween(minDist, dist, maxDist);
}

uint64_t getPrevDist(uint64_t stop, uint64_t dist)
{
  double x = std::max(10.0, (double) stop);
  uint64_t logx = (uint64_t) std::log(x);
  uint64_t minDist = (config::MIN_CACHE_ITERATOR / sizeof(uint64_t)) * logx;
  uint64_t maxDist = (config::MAX_CACHE_ITERATOR / sizeof(uint64_t)) * logx;
  uint64_t tinyDist = PrimeGenerator::maxCachedPrime() * 4;
  uint64_t defaultDist = (uint64_t) (std::sqrt(stop) * 2);

  dist *= 4;
  minDist = inBetween(tinyDist, dist, minDist);
  return inBetween(minDist, defaultDist, maxDist);
}

} // namespace

namespace primesieve {

void IteratorHelper::updateNext(uint64_t& start,
                                uint64_t stopHint,
                                IteratorData& iter)
{
  if (iter.include_start_number)
    start = iter.stop;
  else
    start = checkedAdd(iter.stop, 1);

  iter.include_start_number = false;
  iter.dist = getNextDist(start, iter.dist);

  if (stopHint >= start &&
      stopHint < std::numeric_limits<uint64_t>::max())
  {
    // For primesieve::iterator it is advantageous to buffer
    // slightly more primes than the stopHint since the
    // stopHint is often not 100% accurate and the user
    // might iterate over a few primes > stopHint.
    iter.stop = checkedAdd(stopHint, maxPrimeGap(stopHint));
  }
  else
  {
    // In case the user has used the default stopHint=UINT64_MAX
    // we take a conservative approach and only buffer a
    // small number of primes. If the user uses more primes
    // than we have buffered, then we will increase the sieving
    // distance and buffer more primes (than last time).
    iter.stop = checkedAdd(start, iter.dist);
  }
}

void IteratorHelper::updatePrev(uint64_t& start,
                                uint64_t stopHint,
                                IteratorData& iter)
{
  if (iter.include_start_number)
    iter.stop = start;
  else
    iter.stop = checkedSub(start, 1);

  iter.include_start_number = false;
  iter.dist = getPrevDist(iter.stop, iter.dist);
  start = checkedSub(iter.stop, iter.dist);

  if (stopHint >= start &&
      stopHint <= iter.stop)
    start = checkedSub(stopHint, maxPrimeGap(stopHint));
}

} // namespace
