///
/// @file   PrimeSieve.cpp
/// @brief  The PrimeSieve class provides an easy API for prime
///         sieving (single-threaded).
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include "config.h"
#include "PrimeSieve.h"
#include "primesieve_error.h"
#include "PrimeSieveCallback.h"
#include "PrimeSieve-lock.h"
#include "PrimeFinder.h"
#include "PrimeGenerator.h"
#include "imath.h"

#include <stdint.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <algorithm>

using namespace soe;

const PrimeSieve::SmallPrime PrimeSieve::smallPrimes_[8] =
{
  { 2,  2, 0, "2" },
  { 3,  3, 0, "3" },
  { 5,  5, 0, "5" },
  { 3,  5, 1, "(3, 5)" },
  { 5,  7, 1, "(5, 7)" },
  { 5, 11, 2, "(5, 7, 11)" },
  { 5, 13, 3, "(5, 7, 11, 13)" },
  { 5, 17, 4, "(5, 7, 11, 13, 17)" }
};

PrimeSieve::PrimeSieve() :
  start_(0),
  stop_(0),
  counts_(7),
  flags_(COUNT_PRIMES),
  threadNum_(0),
  parent_(NULL)
{
  setSieveSize(config::SIEVESIZE);
  reset();
}

/// ParallelPrimeSieve creates one PrimeSieve
/// child object for each thread.
///
PrimeSieve::PrimeSieve(PrimeSieve& parent, int threadNum) :
  counts_(7),
  sieveSize_(parent.sieveSize_),
  flags_(parent.flags_),
  threadNum_(threadNum),
  parent_(&parent),
  callback32_(parent.callback32_),
  callback64_(parent.callback64_),
  callback64_tn_(parent.callback64_tn_),
  psc32_(parent.psc32_),
  psc64_(parent.psc64_),
  psc64_tn_(parent.psc64_tn_)
{ }

PrimeSieve::~PrimeSieve()
{ }

std::string PrimeSieve::getVersion()                      { return PRIMESIEVE_VERSION; }
int         PrimeSieve::getMajorVersion()                 { return PRIMESIEVE_MAJOR_VERSION; }
int         PrimeSieve::getMinorVersion()                 { return PRIMESIEVE_MINOR_VERSION; }
uint64_t    PrimeSieve::getStart()                  const { return start_; }
uint64_t    PrimeSieve::getStop()                   const { return stop_; }
uint64_t    PrimeSieve::getInterval()               const { return stop_ - start_; }
uint64_t    PrimeSieve::getPrimeCount()             const { return counts_[0]; }
uint64_t    PrimeSieve::getTwinCount()              const { return counts_[1]; }
uint64_t    PrimeSieve::getTripletCount()           const { return counts_[2]; }
uint64_t    PrimeSieve::getQuadrupletCount()        const { return counts_[3]; }
uint64_t    PrimeSieve::getQuintupletCount()        const { return counts_[4]; }
uint64_t    PrimeSieve::getSextupletCount()         const { return counts_[5]; }
uint64_t    PrimeSieve::getSeptupletCount()         const { return counts_[6]; }
uint64_t    PrimeSieve::getCount(int index)         const { return counts_.at(index); }
double      PrimeSieve::getStatus()                 const { return percent_; }
double      PrimeSieve::getSeconds()                const { return seconds_; }
int         PrimeSieve::getSieveSize()              const { return sieveSize_; }
int         PrimeSieve::getFlags()                  const { return (flags_ & ((1 << 20) - 1)); }
bool        PrimeSieve::isPublicFlags(int flags)    const { return (flags >= 0 && flags < (1 << 20)); }
bool        PrimeSieve::isFlag(int flag)            const { return (flags_ & flag) == flag; }
bool        PrimeSieve::isFlag(int first, int last) const { return (flags_ & (last * 2 - first)) != 0; }
bool        PrimeSieve::isCount(int index)          const { return isFlag(COUNT_PRIMES << index); }
bool        PrimeSieve::isPrint(int index)          const { return isFlag(PRINT_PRIMES << index); }
bool        PrimeSieve::isGenerate()                const { return isFlag(CALLBACK32, CALLBACK64_OBJ_TN); }
bool        PrimeSieve::isCount()                   const { return isFlag(COUNT_PRIMES, COUNT_SEPTUPLETS); }
bool        PrimeSieve::isPrint()                   const { return isFlag(PRINT_PRIMES, PRINT_SEPTUPLETS); }
bool        PrimeSieve::isStatus()                  const { return isFlag(PRINT_STATUS, CALCULATE_STATUS); }
bool        PrimeSieve::isParallelPrimeSieveChild() const { return parent_ != NULL; }

/// Set a start number (lower bound) for sieving.
void PrimeSieve::setStart(uint64_t start)
{
  start_ = start;
}

/// Set a stop number (upper bound) for sieving.
/// @pre stop <= 2^64 - 2^32 * 10
///
void PrimeSieve::setStop(uint64_t stop)
{
  uint64_t maxStop = PrimeFinder::getMaxStop();
  if (stop > maxStop)
    throw primesieve_error("stop must be <= " + PrimeFinder::getMaxStopString());
  stop_ = stop;
}

/// Set the size of the sieve of Eratosthenes array in kilobytes
/// (default = 32). The best sieving performance is achieved with a
/// sieve size of the CPU's L1 data cache size per core.
/// @pre sieveSize >= 1 && <= 2048
///
void PrimeSieve::setSieveSize(int sieveSize)
{
  sieveSize_ = getInBetween(1, floorPowerOf2(sieveSize), 2048);
}

void PrimeSieve::setFlags(int flags)
{
  if (!isPublicFlags(flags))
    throw primesieve_error("invalid flags");
  flags_ = flags;
}

void PrimeSieve::addFlags(int flags)
{
  if (!isPublicFlags(flags))
    throw primesieve_error("invalid flags");
  flags_ |= flags;
}

void PrimeSieve::reset()
{
  std::fill(counts_.begin(), counts_.end(), 0);
  seconds_   = 0.0;
  toUpdate_  = 0;
  processed_ = 0;
  percent_   = -1.0;
}

void PrimeSieve::setLock()
{
  if (isParallelPrimeSieveChild())
    parent_->setLock();
}

void PrimeSieve::unsetLock()
{
  if (isParallelPrimeSieveChild())
    parent_->unsetLock();
}

/// Calculate the sieving status.
/// @param processed  Sum of recently processed segments.
///
bool PrimeSieve::updateStatus(uint64_t processed, bool waitForLock)
{
  if (isParallelPrimeSieveChild()) {
    toUpdate_ += processed;
    if (parent_->updateStatus(toUpdate_, waitForLock))
      toUpdate_ = 0;
  } else {
    processed_ += processed;
    double percent = processed_ * 100.0 / (getInterval() + 1);
    double old = percent_;
    percent_ = std::min(percent, 100.0);
    if (isFlag(PRINT_STATUS))
      printStatus(old, percent_);
  }
  return true;
}

void PrimeSieve::printStatus(double old, double current)
{
  int percent = static_cast<int>(current);
  if (percent > static_cast<int>(old)) {
    std::cout << '\r' << percent << '%' << std::flush;
    if (percent == 100)
      std::cout << std::endl;
  }
}

void PrimeSieve::doSmallPrime(const SmallPrime& sp)
{
  if (sp.firstPrime >= start_ && sp.lastPrime <= stop_) {
    // callback prime numbers
    if (sp.index == 0) {
      if (isFlag(CALLBACK32)) callback32_(sp.firstPrime);
      if (isFlag(CALLBACK64)) callback64_(sp.firstPrime);
      if (isFlag(CALLBACK64_TN)) callback64_tn_(sp.firstPrime, threadNum_);
      if (isFlag(CALLBACK32_OBJ)) psc32_->callback(sp.firstPrime);
      if (isFlag(CALLBACK64_OBJ)) psc64_->callback(sp.firstPrime);
      if (isFlag(CALLBACK64_OBJ_TN)) psc64_tn_->callback(sp.firstPrime, threadNum_);
    }
    if (isCount(sp.index)) counts_[sp.index]++;
    if (isPrint(sp.index)) std::cout << sp.str << '\n';
  }
}

/// Sieve the primes and prime k-tuplets (twin primes, prime
/// triplets, ...) within the interval [start, stop].
///
void PrimeSieve::sieve()
{
  if (start_ > stop_)
    throw primesieve_error("start must be <= stop");
  clock_t t1 = std::clock();
  reset();
  if (isStatus())
    updateStatus(INIT_STATUS, false);

  // Small primes and k-tuplets (first prime <= 5)
  // are checked manually
  if (start_ <= 5) {
    LockGuard lock(*this);
    for (int i = 0; i < 8; i++)
      doSmallPrime(smallPrimes_[i]);
  }
  if (stop_ >= 7) {
    PrimeFinder finder(*this);
    // First generate the sieving primes up to
    // sqrt(stop) and add them to finder
    if (finder.getSqrtStop() > finder.getPreSieve()) {
      PrimeGenerator generator(finder);
      generator.doIt();
    }
    // sieve the primes within [start, stop]
    finder.sieve();
  }

  seconds_ = static_cast<double>(std::clock() - t1) / CLOCKS_PER_SEC;
  if (isStatus())
    updateStatus(FINISH_STATUS, true);
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

/// Generate the primes within the interval [start, stop] and call
/// a callback function for each prime.
///
void PrimeSieve::generatePrimes(uint32_t start,
                                uint32_t stop,
                                void (*callback)(uint32_t))
{
  if (!callback)
    throw primesieve_error("callback must not be NULL");
  callback32_ = callback;
  flags_ = CALLBACK32;
  sieve(start, stop);
}

/// Generate the primes within the interval [start, stop] and call
/// a callback function for each prime.
///
void PrimeSieve::generatePrimes(uint64_t start,
                                uint64_t stop,
                                void (*callback)(uint64_t))
{
  if (!callback)
    throw primesieve_error("callback must not be NULL");
  callback64_ = callback;
  flags_ = CALLBACK64;
  sieve(start, stop);
}

/// Massively parallel (unsynchronized) prime generation method for
/// use with ParallelPrimeSieve.
///
void PrimeSieve::generatePrimes(uint64_t start,
                                uint64_t stop,
                                void (*callback)(uint64_t, int))
{
  if (!callback)
    throw primesieve_error("callback must not be NULL");
  callback64_tn_ = callback;
  flags_ = CALLBACK64_TN;
  sieve(start, stop);
}

/// Generate the primes within the interval [start, stop] and call
/// the callback method of the callbackObject.
///
void PrimeSieve::generatePrimes(uint32_t start,
                                uint32_t stop,
                                PrimeSieveCallback<uint32_t>* callbackObject)
{
  if (!callbackObject)
    throw primesieve_error("callbackObject must not be NULL");
  psc32_ = callbackObject;
  flags_ = CALLBACK32_OBJ;
  sieve(start, stop);
}

/// Generate the primes within the interval [start, stop] and call
/// the callback method of the callbackObject.
///
void PrimeSieve::generatePrimes(uint64_t start,
                                uint64_t stop,
                                PrimeSieveCallback<uint64_t>* callbackObject)
{
  if (!callbackObject)
    throw primesieve_error("callbackObject must not be NULL");
  psc64_ = callbackObject;
  flags_ = CALLBACK64_OBJ;
  sieve(start, stop);
}

/// Massively parallel (unsynchronized) prime generation method for
/// use with ParallelPrimeSieve.
///
void PrimeSieve::generatePrimes(uint64_t start,
                                uint64_t stop,
                                PrimeSieveCallback<uint64_t, int>* callbackObject)
{
  if (!callbackObject)
    throw primesieve_error("callbackObject must not be NULL");
  psc64_tn_ = callbackObject;
  flags_ = CALLBACK64_OBJ_TN;
  sieve(start, stop);
}

// Print member functions

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

void PrimeSieve::printSeptuplets(uint64_t start, uint64_t stop)
{
  sieve(start, stop, PRINT_SEPTUPLETS);
}

// Count member functions

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

uint64_t PrimeSieve::countSeptuplets(uint64_t start, uint64_t stop)
{
  sieve(start, stop, COUNT_SEPTUPLETS);
  return getSeptupletCount();
}
