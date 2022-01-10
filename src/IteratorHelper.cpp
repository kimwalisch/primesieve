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
  uint64_t tinyDist = PrimeGenerator::maxCachedPrime() * 4;
  uint64_t minDist = (uint64_t) std::sqrt(x);
  uint64_t maxDist = 1ull << 60;

  dist *= 4;
  dist = std::max(dist, tinyDist);
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
  dist = std::max(dist, tinyDist);
  dist = std::min(dist, minDist);
  dist = std::max(dist, defaultDist);
  dist = std::min(dist, maxDist);

  return dist;
}

bool useStopHint(uint64_t start,
                 uint64_t stopHint)
{
  return stopHint >= start &&
         stopHint < std::numeric_limits<uint64_t>::max();
}

bool useStopHint(uint64_t start,
                 uint64_t stop,
                 uint64_t stopHint)
{
  return stopHint >= start &&
         stopHint <= stop;
}

} // namespace

namespace primesieve {

void IteratorHelper::next(uint64_t* start,
                          uint64_t* stop,
                          uint64_t stopHint,
                          uint64_t* dist)
{
  *start = checkedAdd(*stop, 1);
  uint64_t maxCachedPrime = PrimeGenerator::maxCachedPrime();

  if (*start < maxCachedPrime)
  {
    // When the stop number <= maxCachedPrime
    // primesieve::iterator uses the primes
    // cache instead of sieving and does not
    // even initialize Erat::init()
    *stop = maxCachedPrime;
    *dist = *stop - *start;
  }
  else
  {
    *dist = getNextDist(*start, *dist);
    *stop = checkedAdd(*start, *dist);

    if (useStopHint(*start, stopHint))
      *stop = checkedAdd(stopHint, maxPrimeGap(stopHint));
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

  if (useStopHint(*start, *stop, stopHint))
    *start = checkedSub(stopHint, maxPrimeGap(stopHint));
}

} // namespace
