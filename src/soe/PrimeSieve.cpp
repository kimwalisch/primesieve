//
// Copyright (c) 2012 Kim Walisch, <kim.walisch@gmail.com>.
// All rights reserved.
//
// This file is part of primesieve.
// Homepage: http://primesieve.googlecode.com
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//   * Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above
//     copyright notice, this list of conditions and the following
//     disclaimer in the documentation and/or other materials provided
//     with the distribution.
//   * Neither the name of the author nor the names of its
//     contributors may be used to endorse or promote products derived
//     from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.

#include "config.h"
#include "PrimeSieve.h"
#include "primesieve_error.h"
#include "PrimeNumberGenerator.h"
#include "PrimeNumberFinder.h"
#include "SynchronizeThreads.h"
#include "imath.h"

#include <stdint.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>

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
  threadNumber_(0),
  parent_(NULL)
{
  setPreSieve(config::PRESIEVE);
  setSieveSize(config::SIEVESIZE);
  reset();
}

/// ParallelPrimeSieve creates one PrimeSieve
/// child object for each thread.
///
PrimeSieve::PrimeSieve(PrimeSieve& parent, int threadNumber) :
  counts_(7),
  preSieve_(parent.preSieve_),
  sieveSize_(parent.sieveSize_),
  flags_(parent.flags_),
  threadNumber_(threadNumber),
  parent_(&parent),
  callback32_(parent.callback32_),
  callback64_(parent.callback64_),
  callback32_obj_(parent.callback32_obj_),
  callback64_obj_(parent.callback64_obj_),
  callback64_int_(parent.callback64_int_),
  obj_(parent.obj_)
{ }

PrimeSieve::~PrimeSieve()
{ }

std::string PrimeSieve::getVersion()                       { return PRIMESIEVE_VERSION; }
int         PrimeSieve::getMajorVersion()                  { return PRIMESIEVE_MAJOR_VERSION; }
int         PrimeSieve::getMinorVersion()                  { return PRIMESIEVE_MINOR_VERSION; }
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
int         PrimeSieve::getPreSieve()               const { return preSieve_; }
int         PrimeSieve::getSieveSize()              const { return sieveSize_; }
int         PrimeSieve::getFlags()                  const { return (flags_ & ((1 << 20) - 1)); }
bool        PrimeSieve::isPublicFlags(int flags)    const { return (flags >= 0 && flags < (1 << 20)); }
bool        PrimeSieve::isFlag(int flag)            const { return (flags_ & flag) == flag; }
bool        PrimeSieve::isFlag(int first, int last) const { return (flags_ & (last * 2 - first)) != 0; }
bool        PrimeSieve::isCount(int index)          const { return isFlag(COUNT_PRIMES << index); }
bool        PrimeSieve::isPrint(int index)          const { return isFlag(PRINT_PRIMES << index); }
bool        PrimeSieve::isCount()                   const { return isFlag(COUNT_PRIMES, COUNT_SEPTUPLETS); }
bool        PrimeSieve::isPrint()                   const { return isFlag(PRINT_PRIMES, PRINT_SEPTUPLETS); }
bool        PrimeSieve::isStatus()                  const { return isFlag(PRINT_STATUS, CALCULATE_STATUS); }
bool        PrimeSieve::isGenerate()                const { return isFlag(CALLBACK32, CALLBACK64_INT) || isPrint(); }
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
  uint64_t maxStop = PrimeNumberFinder::getMaxStop();
  if (stop > maxStop)
    throw primesieve_error("stop must be <= " + PrimeNumberFinder::getMaxStopString());
  stop_ = stop;
}

/// Pre-sieve multiples of small primes <= preSieve (default = 19)
/// to speed up the sieve of Eratosthenes.
/// @pre preSieve >= 13 && <= 23
///
void PrimeSieve::setPreSieve(int preSieve)
{
  preSieve_ = getInBetween(13, preSieve, 23);
}

/// Set the size of the sieve of Eratosthenes array in kilobytes
/// (default = 32). The best sieving performance is achieved with a
/// sieve size of the CPU's L1 data cache size per core.
/// @pre sieveSize >= 1 && <= 4096
///
void PrimeSieve::setSieveSize(int sieveSize)
{
  sieveSize_ = getInBetween(1, floorPowerOf2(sieveSize), 4096);
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
      if (isFlag(CALLBACK32_OBJ)) callback32_obj_(sp.firstPrime, obj_);
      if (isFlag(CALLBACK64_OBJ)) callback64_obj_(sp.firstPrime, obj_);
      if (isFlag(CALLBACK64_INT)) callback64_int_(sp.firstPrime, threadNumber_);
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
    SynchronizeThreads lock(*this);
    for (int i = 0; i < 8; i++)
      doSmallPrime(smallPrimes_[i]);
  }
  if (stop_ >= 7) {
    // fast segmented SieveOfEratosthenes object that sieves
    // the primes within the interval [start, stop]
    PrimeNumberFinder finder(*this);
    if (finder.getSqrtStop() > finder.getPreSieve()) {
      // generates the primes up to sqrt(stop)
      // needed for sieving by finder
      PrimeNumberGenerator generator(finder);
      // tiny sieve of Eratosthenes that generates the primes up
      // to stop_^0.25 needed for sieving by generator
      uint_t N = generator.getSqrtStop();
      std::vector<uint8_t> isPrime(N / 8 + 1, 0xAA);
      for (uint_t i = 3; i * i <= N; i += 2) {
        if (isPrime[i >> 3] & (1 << (i & 7)))
          for (uint_t j = i * i; j <= N; j += i + i)
            isPrime[j >> 3] &= ~(1 << (j & 7));
      }
      for (uint_t i = generator.getPreSieve() + 1; i <= N; i++) {
        if (isPrime[i >> 3] & (1 << (i & 7)))
          generator.sieve(i);
      }
      generator.finish();
    }
    finder.finish();
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
  if (callback == NULL)
    throw primesieve_error("callback must not be NULL");
  callback32_ = callback;
  flags_ = CALLBACK32;
  setPreSieve(17);
  sieve(start, stop);
}

/// Generate the primes within the interval [start, stop] and call
/// a callback function for each prime.
///
void PrimeSieve::generatePrimes(uint64_t start,
                                uint64_t stop,
                                void (*callback)(uint64_t))
{
  if (callback == NULL)
    throw primesieve_error("callback must not be NULL");
  callback64_ = callback;
  flags_ = CALLBACK64;
  setPreSieve(17);
  sieve(start, stop);
}

/// Generate the primes within the interval [start, stop] and call
/// an OOP-style callback function for each prime.
///
void PrimeSieve::generatePrimes(uint32_t start,
                                uint32_t stop,
                                void (*callback)(uint32_t, void*), void* obj)
{
  if (callback == NULL)
    throw primesieve_error("callback must not be NULL");
  callback32_obj_ = callback;
  obj_ = obj;
  flags_ = CALLBACK32_OBJ;
  setPreSieve(17);
  sieve(start, stop);
}

/// Generate the primes within the interval [start, stop] and call
/// an OOP-style callback function for each prime.
///
void PrimeSieve::generatePrimes(uint64_t start,
                                uint64_t stop,
                                void (*callback)(uint64_t, void*), void* obj)
{
  if (callback == NULL)
    throw primesieve_error("callback must not be NULL");
  callback64_obj_ = callback;
  obj_ = obj;
  flags_ = CALLBACK64_OBJ;
  setPreSieve(17);
  sieve(start, stop);
}

/// Unsynchronized prime generation method for use with
/// ParallelPrimeSieve.
///
void PrimeSieve::generatePrimes(uint64_t start,
                                uint64_t stop,
                                void (*callback)(uint64_t, int))
{
  if (callback == NULL)
    throw primesieve_error("callback must not be NULL");
  callback64_int_ = callback;
  flags_ = CALLBACK64_INT;
  setPreSieve(17);
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
