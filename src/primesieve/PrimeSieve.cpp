///
/// @file   PrimeSieve.hpp
/// @brief  The PrimeSieve class is a high level class that
///         manages prime sieving using the PreSieve, SievingPrimes
///         and PrimeGenerator classes.
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/config.hpp>
#include <primesieve/PrimeSieve.hpp>
#include <primesieve/primesieve_error.hpp>
#include <primesieve/PreSieve.hpp>
#include <primesieve/PrimeGenerator.hpp>
#include <primesieve/SievingPrimes.hpp>
#include <primesieve/StorePrimes.hpp>
#include <primesieve/pmath.hpp>

#include <stdint.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
#include <string>

using namespace std;

namespace {

struct SmallPrime
{
  uint64_t first;
  uint64_t last;
  int index;
  string str;
};

const array<SmallPrime, 8> smallPrimes
{{
  { 2,  2, 0, "2" },
  { 3,  3, 0, "3" },
  { 5,  5, 0, "5" },
  { 3,  5, 1, "(3, 5)" },
  { 5,  7, 1, "(5, 7)" },
  { 5, 11, 2, "(5, 7, 11)" },
  { 5, 13, 3, "(5, 7, 11, 13)" },
  { 5, 17, 4, "(5, 7, 11, 13, 17)" }
}};

} // namespace

namespace primesieve {

PrimeSieve::PrimeSieve() :
  start_(0),
  stop_(0),
  counts_(6),
  flags_(COUNT_PRIMES),
  parent_(nullptr),
  store_(nullptr)
{
  setSieveSize(L1_DCACHE_SIZE);
  reset();
}

/// ParallelPrimeSieve creates one PrimeSieve
/// child object for each thread.
///
PrimeSieve::PrimeSieve(PrimeSieve* parent) :
  counts_(6),
  sieveSize_(parent->sieveSize_),
  flags_(parent->flags_),
  parent_(parent),
  store_(parent->store_)
{ }

PrimeSieve::~PrimeSieve()
{ }

void PrimeSieve::reset()
{
  fill(counts_.begin(), counts_.end(), 0);
  seconds_ = 0.0;
  toUpdate_ = 0;
  processed_ = 0;
  percent_ = -1.0;
}

uint64_t PrimeSieve::getStart()                  const { return start_; }
uint64_t PrimeSieve::getStop()                   const { return stop_; }
uint64_t PrimeSieve::getDistance()               const { return stop_ - start_; }
uint64_t PrimeSieve::getPrimeCount()             const { return counts_[0]; }
uint64_t PrimeSieve::getTwinCount()              const { return counts_[1]; }
uint64_t PrimeSieve::getTripletCount()           const { return counts_[2]; }
uint64_t PrimeSieve::getQuadrupletCount()        const { return counts_[3]; }
uint64_t PrimeSieve::getQuintupletCount()        const { return counts_[4]; }
uint64_t PrimeSieve::getSextupletCount()         const { return counts_[5]; }
uint64_t PrimeSieve::getCount(int index)         const { return counts_.at(index); }
double   PrimeSieve::getStatus()                 const { return percent_; }
double   PrimeSieve::getSeconds()                const { return seconds_; }
int      PrimeSieve::getSieveSize()              const { return sieveSize_; }
bool     PrimeSieve::isFlag(int flag)            const { return (flags_ & flag) == flag; }
bool     PrimeSieve::isFlag(int first, int last) const { return (flags_ & (last * 2 - first)) != 0; }
bool     PrimeSieve::isCount(int index)          const { return isFlag(COUNT_PRIMES << index); }
bool     PrimeSieve::isPrint(int index)          const { return isFlag(PRINT_PRIMES << index); }
bool     PrimeSieve::isCount()                   const { return isFlag(COUNT_PRIMES, COUNT_SEXTUPLETS); }
bool     PrimeSieve::isPrint()                   const { return isFlag(PRINT_PRIMES, PRINT_SEXTUPLETS); }
bool     PrimeSieve::isStatus()                  const { return isFlag(PRINT_STATUS, CALCULATE_STATUS); }
bool     PrimeSieve::isStore()                   const { return store_ != nullptr; }
bool     PrimeSieve::isParallelPrimeSieve()      const { return parent_ != nullptr; }

/// Set a start number (lower bound) for sieving
void PrimeSieve::setStart(uint64_t start)
{
  start_ = start;
}

/// Set a stop number (upper bound) for sieving
void PrimeSieve::setStop(uint64_t stop)
{
  stop_ = stop;
}

/// Set the size of the sieve of Eratosthenes array in kilobytes
/// (default = 32). The best sieving performance is achieved with a
/// sieve size of the CPU's L1 data cache size per core.
///
void PrimeSieve::setSieveSize(int sieveSize)
{
  sieveSize_ = inBetween(1, floorPowerOf2(sieveSize), 2048);
}

Store& PrimeSieve::getStore()
{
  return *store_;
}

PrimeSieve::counts_t& PrimeSieve::getCounts()
{
  return counts_;
}

void PrimeSieve::setFlags(int flags)
{
  flags_ = flags;
}

void PrimeSieve::addFlags(int flags)
{
  flags_ |= flags;
}

/// Print status in percent to stdout.
/// @processed:  Sum of recently processed segments
///
bool PrimeSieve::updateStatus(uint64_t processed, bool tryLock)
{
  if (isParallelPrimeSieve())
  {
    toUpdate_ += processed;
    if (parent_->updateStatus(toUpdate_, tryLock))
      toUpdate_ = 0;
  }
  else
  {
    processed_ += processed;
    double percent = 100;
    if (getDistance() > 0)
      percent = processed_ * 100.0 / getDistance();
    double old = percent_;
    percent_ = min(percent, 100.0);
    if (isFlag(PRINT_STATUS))
      printStatus(old, percent_);
  }
  return true;
}

void PrimeSieve::printStatus(double old, double current)
{
  int percent = (int) current;
  if (percent > (int) old)
  {
    cout << '\r' << percent << '%' << flush;
    if (percent == 100)
      cout << endl;
  }
}

/// Process small primes <= 5 and small k-tuplets <= 17
void PrimeSieve::processSmallPrimes()
{
  for (auto& p : smallPrimes)
  {
    if (p.first >= start_ && p.last <= stop_)
    {
      if (isCount(p.index))
        counts_[p.index]++;
      if (isPrint(p.index))
        cout << p.str << '\n';
      if (isStore() && p.index == 0)
        (*store_)(p.first);
    }
  }
}

/// Sieve the primes and prime k-tuplets (twin primes,
/// prime triplets, ...) in [start, stop]
///
void PrimeSieve::sieve()
{
  reset();

  if (start_ > stop_)
    return;

  auto t1 = chrono::system_clock::now();
  int initStatus = 0;
  int finishStatus = 10;

  if (isStatus())
    updateStatus(initStatus);

  if (start_ <= 5)
    processSmallPrimes();

  if (stop_ >= 7)
  {
    PreSieve preSieve(start_, stop_);
    PrimeGenerator primeGen(*this, preSieve);

    // generate sieving primes for primeGen
    if (primeGen.getSqrtStop() > preSieve.getLimit())
    {
      SievingPrimes sp(primeGen, preSieve);
      sp.generate();
    }

    // sieve [start, stop]
    primeGen.sieve();
  }

  auto t2 = chrono::system_clock::now();
  chrono::duration<double> seconds = t2 - t1;
  seconds_ = seconds.count();

  if (isStatus())
    updateStatus(finishStatus, false);
}

void PrimeSieve::sieve(uint64_t start, uint64_t stop)
{
  setStart(start);
  setStop(stop);
  sieve();
}

void PrimeSieve::sieve(uint64_t start, uint64_t stop, int flags)
{
  setStart(start);
  setStop(stop);
  setFlags(flags);
  sieve();
}

void PrimeSieve::storePrimes(uint64_t start, uint64_t stop, Store* store)
{
  if (!store)
    throw primesieve_error("invalid store pointer");
  store_ = store;
  flags_ = 0;
  sieve(start, stop);
}

// Print methods

void PrimeSieve::printPrimes(uint64_t start, uint64_t stop)
{
  sieve(start, stop, PRINT_PRIMES);
}

void PrimeSieve::printTwins(uint64_t start, uint64_t stop)
{
  sieve(start, stop, PRINT_TWINS);
}

void PrimeSieve::printTriplets(uint64_t start, uint64_t stop)
{
  sieve(start, stop, PRINT_TRIPLETS);
}

void PrimeSieve::printQuadruplets(uint64_t start, uint64_t stop)
{
  sieve(start, stop, PRINT_QUADRUPLETS);
}

void PrimeSieve::printQuintuplets(uint64_t start, uint64_t stop)
{
  sieve(start, stop, PRINT_QUINTUPLETS);
}

void PrimeSieve::printSextuplets(uint64_t start, uint64_t stop)
{
  sieve(start, stop, PRINT_SEXTUPLETS);
}

// Count methods

uint64_t PrimeSieve::countPrimes(uint64_t start, uint64_t stop)
{
  sieve(start, stop, COUNT_PRIMES);
  return getPrimeCount();
}

uint64_t PrimeSieve::countTwins(uint64_t start, uint64_t stop)
{
  sieve(start, stop, COUNT_TWINS);
  return getTwinCount();
}

uint64_t PrimeSieve::countTriplets(uint64_t start, uint64_t stop)
{
  sieve(start, stop, COUNT_TRIPLETS);
  return getTripletCount();
}

uint64_t PrimeSieve::countQuadruplets(uint64_t start, uint64_t stop)
{
  sieve(start, stop, COUNT_QUADRUPLETS);
  return getQuadrupletCount();
}

uint64_t PrimeSieve::countQuintuplets(uint64_t start, uint64_t stop)
{
  sieve(start, stop, COUNT_QUINTUPLETS);
  return getQuintupletCount();
}

uint64_t PrimeSieve::countSextuplets(uint64_t start, uint64_t stop)
{
  sieve(start, stop, COUNT_SEXTUPLETS);
  return getSextupletCount();
}

} // namespace
