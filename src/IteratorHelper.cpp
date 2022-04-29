///
/// @file  IteratorHelper.cpp
///        Functions used to calculate the next start and stop
///        numbers for primesieve::iterator.
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/config.hpp>
#include <primesieve/IteratorHelper.hpp>
#include <primesieve/PrimeGenerator.hpp>
#include <primesieve/pmath.hpp>

#include <stdint.h>
#include <algorithm>
#include <cmath>
#include <limits>

using namespace primesieve;

namespace {

uint64_t getNextDist(uint64_t n, uint64_t dist)
{
  double x = (double) n;
  uint64_t minDist = (uint64_t) std::sqrt(x);
  uint64_t maxDist = 1ull << 60;

  dist *= 4;
  dist = std::max(dist, PrimeGenerator::maxCachedPrime());
  dist = std::max(dist, minDist);
  dist = std::min(dist, maxDist);

  return dist;
}

uint64_t getPrevDist(uint64_t n, uint64_t dist)
{
  double x = (double) n;
  x = std::max(x, 10.0);
  double logx = std::ceil(std::log(x));

  uint64_t minDist = config::MIN_CACHE_ITERATOR;
  uint64_t maxDist = config::MAX_CACHE_ITERATOR;
  minDist /= sizeof(uint64_t);
  maxDist /= sizeof(uint64_t);
  minDist *= (uint64_t) logx;
  maxDist *= (uint64_t) logx;

  uint64_t tinyDist = PrimeGenerator::maxCachedPrime() * 4;
  uint64_t defaultDist = (uint64_t) (std::sqrt(x) * 2);

  dist *= 4;
  dist = inBetween(tinyDist, dist, minDist);
  dist = std::max(dist, defaultDist);
  dist = std::min(dist, maxDist);

  return dist;
}

} // namespace

namespace primesieve {

void IteratorHelper::next(uint64_t* start,
                          uint64_t* stop,
                          uint64_t stopHint,
                          uint64_t* dist)
{
  *start = checkedAdd(*stop, 1);
  *dist = getNextDist(*start, *dist);

  if (stopHint >= *start &&
      stopHint < std::numeric_limits<uint64_t>::max())
  {
    // For primesieve::iterator it is advantageous to buffer
    // slightly more primes than the stopHint since the
    // stopHint is often not 100% accurate and the user
    // might iterate over a few primes > stopHint.
    *stop = checkedAdd(stopHint, maxPrimeGap(stopHint));
  }
  else
  {
    // In case the user has used the default stopHint=UINT64_MAX
    // we take a conservative approach and only buffer a
    // small number of primes. If the user uses more primes
    // than we have buffered, then we will increase the sieving
    // distance and buffer more primes (than last time).
    *stop = checkedAdd(*start, *dist);
  }
}

void IteratorHelper::prev(uint64_t* start,
                          uint64_t* stop,
                          uint64_t stopHint,
                          uint64_t* dist)
{
  *stop = checkedSub(*start, 1);
  *dist = getPrevDist(*stop, *dist);
  *start = checkedSub(*stop, *dist);

  if (stopHint >= *start &&
      stopHint <= *stop)
    *start = checkedSub(stopHint, maxPrimeGap(stopHint));
}

} // namespace
