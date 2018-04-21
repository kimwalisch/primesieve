///
/// @file  IteratorHelper.cpp
///        Functions used to calculate the next start and stop
///        numbers for primesieve::iterator.
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/config.hpp>
#include <primesieve/IteratorHelper.hpp>
#include <primesieve/pmath.hpp>

#include <stdint.h>
#include <algorithm>
#include <cmath>
#include <limits>

using namespace std;
using namespace primesieve;

namespace {

uint64_t getNextDist(uint64_t n, uint64_t dist)
{
  double x = (double) n;
  x = max(x, 16.0);
  x = sqrt(x) / log(log(x));

  uint64_t minDist = (uint64_t) x;
  uint64_t limit = numeric_limits<uint64_t>::max() / 4;
  dist = max(dist, minDist);

  if (dist < limit)
    dist *= 4;

  return dist;
}

uint64_t getPrevDist(uint64_t n, uint64_t* dist)
{
  double x = (double) n;
  x = max(x, 10.0);

  double minDist = config::MIN_CACHE_ITERATOR;
  double maxDist = config::MAX_CACHE_ITERATOR;
  double logx = log(x);

  minDist *= logx;
  maxDist *= logx;

  minDist /= sizeof(uint64_t);
  maxDist /= sizeof(uint64_t);

  if (*dist < minDist)
  {
    minDist = (double) *dist;
    *dist *= 4;
  }

  double defaultDist = sqrt(x) * 2;
  double newDist = max(minDist, defaultDist);
  newDist = min(newDist, maxDist);

  return (uint64_t) newDist;
}

bool useStopHint(uint64_t start,
                 uint64_t stopHint)
{
  return stopHint >= start &&
         stopHint < numeric_limits<uint64_t>::max();
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
  *dist = getNextDist(*start, *dist);
  *stop = checkedAdd(*start, *dist);

  if (useStopHint(*start, stopHint))
    *stop = checkedAdd(stopHint, maxPrimeGap(stopHint));
}

void IteratorHelper::prev(uint64_t* start,
                          uint64_t* stop,
                          uint64_t stopHint,
                          uint64_t* dist)
{
  *stop = checkedSub(*start, 1);
  uint64_t prevDist = getPrevDist(*stop, dist);
  *start = checkedSub(*stop, prevDist);

  if (useStopHint(*start, *stop, stopHint))
    *start = checkedSub(stopHint, maxPrimeGap(stopHint));
}

} // namespace
