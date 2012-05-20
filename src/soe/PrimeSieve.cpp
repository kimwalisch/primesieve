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

#include "PrimeSieve.h"
#include "PrimeNumberFinder.h"
#include "PrimeNumberGenerator.h"
#include "config.h"
#include "imath.h"
#include "PreSieve.h"

#include <stdint.h>
#include <stdexcept>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>

using namespace soe;

PrimeSieve::PrimeSieve() :
  start_(0),
  stop_(0),
  flags_(COUNT_PRIMES),
  parent_(NULL)
{
  setPreSieveLimit(config::PRESIEVE_LIMIT);
  setSieveSize(config::SIEVESIZE);
  reset();
}

/**
 * Used by ParallelPrimeSieve which uses multiple PrimeSieve objects
 * and threads to sieve primes in parallel.
 * @see ParallelPrimeSieve::sieve()
 */
PrimeSieve::PrimeSieve(PrimeSieve* parent) :
  preSieveLimit_(parent->preSieveLimit_),
  sieveSize_(parent->sieveSize_),
  flags_(parent->flags_),
  parent_(parent),
  callback32_(parent->callback32_),
  callback64_(parent->callback64_),
  callback32_OOP_(parent->callback32_OOP_),
  callback64_OOP_(parent->callback64_OOP_),
  obj_(parent->obj_)
{ }

void PrimeSieve::reset() {
  sumSegments_ = 0;
  for (int i = 0; i < COUNTS_SIZE; i++)
    counts_[i] = 0;
  interval_ = static_cast<double>(stop_ - start_ + 1);
  status_ = -1.0;
  timeElapsed_ = 0.0;
  if (isStatus())
    updateStatus(0);
}

uint64_t PrimeSieve::getStart()         const { return start_; }
uint64_t PrimeSieve::getStop()          const { return stop_; }
uint32_t PrimeSieve::getSieveSize()     const { return sieveSize_; }
uint32_t PrimeSieve::getPreSieveLimit() const { return preSieveLimit_; }

/** Get the count of primes after sieve() execution. */
uint64_t PrimeSieve::getPrimeCount()      const { return counts_[0]; }
uint64_t PrimeSieve::getTwinCount()       const { return counts_[1]; }
uint64_t PrimeSieve::getTripletCount()    const { return counts_[2]; }
uint64_t PrimeSieve::getQuadrupletCount() const { return counts_[3]; }
uint64_t PrimeSieve::getQuintupletCount() const { return counts_[4]; }
uint64_t PrimeSieve::getSextupletCount()  const { return counts_[5]; }
uint64_t PrimeSieve::getSeptupletCount()  const { return counts_[6]; }

uint64_t PrimeSieve::getCounts(uint32_t index) const {
  if (index >= COUNTS_SIZE)
    throw std::out_of_range("getCounts(uint32_t) index out of range");
  return counts_[index];
}

/** Get the current status in percent of sieve(). */
double PrimeSieve::getStatus() const {
  return status_;
}

/** Get the time elapsed in seconds of sieve(). */
double PrimeSieve::getTimeElapsed() const {
  return timeElapsed_;
}

/** Get the current set public flags. */
uint32_t PrimeSieve::getFlags() const {
  return flags_ & ((1U << 20) - 1);
}

bool PrimeSieve::isFlag(uint32_t flag) const {
  return (flags_ & flag) == flag; 
}

/** @return true  If any bit in the range [first, last] is set. */
bool PrimeSieve::isFlag(uint32_t first, uint32_t last) const {
  return (flags_ & (last * 2 - first)) != 0;
}

bool PrimeSieve::isCount() const {
  return isFlag(COUNT_PRIMES, COUNT_SEPTUPLETS);
}

/** For convenience the PRINT_STATUS flag is not included. */
bool PrimeSieve::isPrint() const {
  return isFlag(PRINT_PRIMES, PRINT_SEPTUPLETS);
}

bool PrimeSieve::isGenerate() const {
  return isFlag(CALLBACK32_PRIMES, CALLBACK64_OOP_PRIMES) || isPrint();
}

bool PrimeSieve::isStatus() const {
  return isFlag(CALCULATE_STATUS, PRINT_STATUS);
}

/**
 * Set a start number for sieving.
 * @pre start < (2^64-1) - (2^32-1) * 10
 */
void PrimeSieve::setStart(uint64_t start) {
  if (start >= UINT64_MAX - UINT32_MAX * UINT64_C(10))
    throw std::invalid_argument("START must be < (2^64-1) - (2^32-1) * 10");
  start_ = start;
}

/**
 * Set a stop number for sieving.
 * @pre stop < (2^64-1) - (2^32-1) * 10
 */
void PrimeSieve::setStop(uint64_t stop) {
  if (stop >= UINT64_MAX - UINT32_MAX * UINT64_C(10))
    throw std::invalid_argument("STOP must be < (2^64-1) - (2^32-1) * 10");
  stop_ = stop;
}

/**
 * Multiples of small primes <= preSieveLimit are pre-sieved
 * to speed up the sieve of Eratosthenes.
 * @param preSieveLimit  >= 13 && <= 23, default = 19
 *                       min = 13 (uses 1001 bytes)
 *                       max = 23 (uses 7 megabytes)
 */
void PrimeSieve::setPreSieveLimit(uint32_t preSieveLimit) {
  preSieveLimit_ = getBoundedValue<uint32_t>(13, preSieveLimit, 23);
}

/**
 * Set the size of the sieve of Eratosthenes array in kilobytes.
 * The best performance is achieved with a sieve size of the CPU's L1
 * data cache size (usually 32 or 64 KB) when sieving < 10^15 and a
 * sieve size of the CPU's L2 cache size above.
 * @param sieveSize  >= 1 && <= 4096 kilobytes, default = 32, 
 *                   sieveSize is rounded up to the next highest power
 *                   of 2.
 */
void PrimeSieve::setSieveSize(uint32_t sieveSize) {
  sieveSize_ = getBoundedValue<uint32_t>(1, nextPowerOf2(sieveSize), 4096);
}

/**
 * Settings for sieve().
 * @see primesieve/docs/USAGE_EXAMPLES
 * @param flags
 *   PrimeSieve::COUNT_PRIMES      OR (bitwise '|')
 *   PrimeSieve::COUNT_TWINS       OR
 *   PrimeSieve::COUNT_TRIPLETS    OR
 *   PrimeSieve::COUNT_QUADRUPLETS OR
 *   PrimeSieve::COUNT_QUINTUPLETS OR
 *   PrimeSieve::COUNT_SEXTUPLETS  OR
 *   PrimeSieve::COUNT_SEPTUPLETS  OR
 *   PrimeSieve::PRINT_PRIMES      OR
 *   PrimeSieve::PRINT_TWINS       OR
 *   PrimeSieve::PRINT_TRIPLETS    OR
 *   PrimeSieve::PRINT_QUADRUPLETS OR
 *   PrimeSieve::PRINT_QUINTUPLETS OR
 *   PrimeSieve::PRINT_SEXTUPLETS  OR
 *   PrimeSieve::PRINT_SEPTUPLETS  OR
 *   PrimeSieve::CALCULATE_STATUS  OR
 *   PrimeSieve::PRINT_STATUS.
 */
void PrimeSieve::setFlags(uint32_t flags) {
  if (flags >= (1U << 20))
    throw std::invalid_argument("invalid flags");
  flags_ = flags;
}

void PrimeSieve::addFlags(uint32_t flags) {
  if (flags >= (1U << 20))
    throw std::invalid_argument("invalid flags");
  flags_ |= flags;
}

void PrimeSieve::set_lock() {
  if (parent_ != NULL)
    parent_->set_lock();
}

void PrimeSieve::unset_lock() {
  if (parent_ != NULL)
    parent_->unset_lock();
}

/**
 * Calculate the current status in percent of sieve()
 * and print it to the standard output.
 * @param segment  The interval size of the processed segment.
 */
void PrimeSieve::updateStatus(uint32_t segment) {
  if (parent_ != NULL)
    parent_->updateStatus(segment);
  else {
    sumSegments_ += segment;
    int old = static_cast<int>(status_);
    status_ = std::min((sumSegments_ / interval_) * 100.0, 100.0);
    if (isFlag(PRINT_STATUS)) {
      int status = static_cast<int>(status_);
      if (status > old)
        std::cout << '\r' << status << '%' << std::flush;
    }
  }
}

void PrimeSieve::doSmallPrime(uint32_t minPrime,
                              uint32_t maxPrime,
                              uint32_t index,
                              const std::string& primeStr)
{
  if (start_ <= minPrime && maxPrime <= stop_) {
    // only one thread at a time may access this code section
    LockGuard lock(*this);
    if (index == 0) {
      if (isFlag(CALLBACK32_PRIMES)) callback32_(minPrime);
      if (isFlag(CALLBACK64_PRIMES)) callback64_(minPrime);
      if (isFlag(CALLBACK32_OOP_PRIMES)) callback32_OOP_(minPrime, obj_);
      if (isFlag(CALLBACK64_OOP_PRIMES)) callback64_OOP_(minPrime, obj_);
    }
    if (isFlag(COUNT_PRIMES << index)) counts_[index]++;
    if (isFlag(PRINT_PRIMES << index)) std::cout << primeStr << '\n';
  }
}

/**
 * Sieve the primes and prime k-tuplets (twins, triplets, ...) within
 * the interval [start_, stop_] using PrimeSieve's fast segmented
 * sieve of Eratosthenes implementation.
 */
void PrimeSieve::sieve() {
  if (stop_ < start_)
    throw std::invalid_argument("STOP must be >= START");
  clock_t t1 = std::clock();
  reset();

  // do small primes and k-tuplets manually
  if (start_ <= 5) {
    doSmallPrime(2,  2, 0, "2");
    doSmallPrime(3,  3, 0, "3");
    doSmallPrime(5,  5, 0, "5");
    doSmallPrime(3,  5, 1, "(3, 5)");
    doSmallPrime(5,  7, 1, "(5, 7)");
    doSmallPrime(5, 11, 2, "(5, 7, 11)");
    doSmallPrime(5, 13, 3, "(5, 7, 11, 13)");
    doSmallPrime(5, 17, 4, "(5, 7, 11, 13, 17)");
  }

  if (stop_ >= 7) {
    // fast segmented SieveOfEratosthenes object that
    // sieves the primes within [start_, stop_]
    PrimeNumberFinder finder(*this);
    if (finder.needGenerator()) {
      // fast segmented SieveOfEratosthenes object that generates the
      // primes up to sqrt(stop_) needed for sieving by 'finder'
      PrimeNumberGenerator generator(finder);
      // tiny sieve of Eratosthenes implementation that generates the
      // primes up to stop_^0.25 needed for sieving by 'generator'
      uint32_t N = generator.getSquareRoot();
      std::vector<uint32_t> isPrime(N / 32 + 1, 0xAAAAAAAAU);
      for (uint32_t i = 3; i * i <= N; i += 2) {
        if (isPrime[i >> 5] & (1 << (i & 31)))
          for (uint32_t j = i * i; j <= N; j += i * 2)
            isPrime[j >> 5] &= ~(1 << (j & 31));
      }
      for (uint32_t i = generator.getPreSieveLimit() + 1; i <= N; i++) {
        if (isPrime[i >> 5] & (1 << (i & 31)))
          generator.sieve(i);
      }
      generator.finish();
    }
    finder.finish();
  }

  timeElapsed_ = static_cast<double>(std::clock() - t1) / CLOCKS_PER_SEC;
  if (isStatus())
    updateStatus(10);
}

/**
 * Convenience sieve member functions.
 */

 void PrimeSieve::sieve(uint64_t start, uint64_t stop) {
  setStart(start);
  setStop(stop);
  sieve();
}

void PrimeSieve::sieve(uint64_t start, uint64_t stop, uint32_t flags) {
  setStart(start);
  setStop(stop);
  setFlags(flags);
  sieve();
}

/**
 * 32 & 64-bit prime number generation methods, see
 * /docs/USAGE_EXAMPLES for usage examples.
 */

void PrimeSieve::generatePrimes(uint32_t start, 
                                uint32_t stop,
                                void (*callback)(uint32_t)) {
  if (callback == NULL)
    throw std::invalid_argument("callback must not be NULL");
  callback32_ = callback;
  flags_ = CALLBACK32_PRIMES;
  // speed up initialization (default pre-sieve limit = 19)
  setPreSieveLimit(13);
  sieve(start, stop);
}

void PrimeSieve::generatePrimes(uint64_t start, 
                                uint64_t stop,
                                void (*callback)(uint64_t)) {
  if (callback == NULL)
    throw std::invalid_argument("callback must not be NULL");
  callback64_ = callback;
  flags_ = CALLBACK64_PRIMES;
  setPreSieveLimit(13);
  sieve(start, stop);
}

void PrimeSieve::generatePrimes(uint32_t start, 
                                uint32_t stop,
                                void (*callback)(uint32_t, void*), void* obj) {
  if (callback == NULL || obj == NULL)
    throw std::invalid_argument("callback & obj must not be NULL");
  callback32_OOP_ = callback;
  obj_ = obj;
  flags_ = CALLBACK32_OOP_PRIMES;
  setPreSieveLimit(13);
  sieve(start, stop);
}

void PrimeSieve::generatePrimes(uint64_t start, 
                                uint64_t stop,
                                void (*callback)(uint64_t, void*), void* obj) {
  if (callback == NULL || obj == NULL)
    throw std::invalid_argument("callback & obj must not be NULL");
  callback64_OOP_ = callback;
  obj_ = obj;
  flags_ = CALLBACK64_OOP_PRIMES;
  setPreSieveLimit(13);
  sieve(start, stop);
}

/**
 * Convenience count member functions.
 */

uint64_t PrimeSieve::getPrimeCount(uint64_t start, uint64_t stop) {
  sieve(start, stop, COUNT_PRIMES);
  return getPrimeCount();
}

uint64_t PrimeSieve::getTwinCount(uint64_t start, uint64_t stop) {
  sieve(start, stop, COUNT_TWINS);
  return getTwinCount();
}

uint64_t PrimeSieve::getTripletCount(uint64_t start, uint64_t stop) {
  sieve(start, stop, COUNT_TRIPLETS);
  return getTripletCount();
}

uint64_t PrimeSieve::getQuadrupletCount(uint64_t start, uint64_t stop) {
  sieve(start, stop, COUNT_QUADRUPLETS);
  return getQuadrupletCount();
}

uint64_t PrimeSieve::getQuintupletCount(uint64_t start, uint64_t stop) {
  sieve(start, stop, COUNT_QUINTUPLETS);
  return getQuintupletCount();
}

uint64_t PrimeSieve::getSextupletCount(uint64_t start, uint64_t stop) {
  sieve(start, stop, COUNT_SEXTUPLETS);
  return getSextupletCount();
}

uint64_t PrimeSieve::getSeptupletCount(uint64_t start, uint64_t stop) {
  sieve(start, stop, COUNT_SEPTUPLETS);
  return getSeptupletCount();
}

/**
 * Convenience print member functions (to std::cout).
 */

void PrimeSieve::printPrimes(uint64_t start, uint64_t stop) {
  sieve(start, stop, PRINT_PRIMES);
}

void PrimeSieve::printTwins(uint64_t start, uint64_t stop) {
  sieve(start, stop, PRINT_TWINS);
}

void PrimeSieve::printTriplets(uint64_t start, uint64_t stop) {
  sieve(start, stop, PRINT_TRIPLETS);
}

void PrimeSieve::printQuadruplets(uint64_t start, uint64_t stop) {
  sieve(start, stop, PRINT_QUADRUPLETS);
}

void PrimeSieve::printQuintuplets(uint64_t start, uint64_t stop) {
  sieve(start, stop, PRINT_QUINTUPLETS);
}

void PrimeSieve::printSextuplets(uint64_t start, uint64_t stop) {
  sieve(start, stop, PRINT_SEXTUPLETS);
}

void PrimeSieve::printSeptuplets(uint64_t start, uint64_t stop) {
  sieve(start, stop, PRINT_SEPTUPLETS);
}
