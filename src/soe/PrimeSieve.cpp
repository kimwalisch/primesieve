//
// Copyright (c) 2011 Kim Walisch, <kim.walisch@gmail.com>.
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
#include "ParallelPrimeSieve.h"
#include "PrimeNumberFinder.h"
#include "PrimeNumberGenerator.h"
#include "defs.h"
#include "bithacks.h"
#include "PreSieve.h"

#include <stdexcept>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>

#if defined(_OPENMP)
  #include <omp.h>
#endif

PrimeSieve::PrimeSieve() :
  startNumber_(0),
  stopNumber_(0),
  flags_(COUNT_PRIMES)
{
  parent_ = this;
  this->setSieveSize(defs::PRIMESIEVE_SIEVESIZE);
  this->setPreSieveLimit(defs::PRIMESIEVE_PRESIEVE_LIMIT);
  this->reset();
}

/**
 * ParallelPrimeSieve uses multiple PrimeSieve objects and threads to
 * sieve primes in parallel.
 * @see ParallelPrimeSieve::sieve()
 */
PrimeSieve::PrimeSieve(uint64_t startNumber,
                       uint64_t stopNumber, 
                       ParallelPrimeSieve* parent) :
  startNumber_(startNumber),
  stopNumber_(stopNumber),
  sieveSize_(parent->sieveSize_),
  preSieveLimit_(parent->preSieveLimit_),
  flags_(parent->flags_),
  parent_(parent)
{
  if (testFlags(CALLBACK_FLAGS)) {
    callback32_     = parent->callback32_;
    callback32_OOP_ = parent->callback32_OOP_;
    callback64_     = parent->callback64_;
    callback64_OOP_ = parent->callback64_OOP_;
    cbObj_          = parent->cbObj_;
  }
  this->reset();
}

uint64_t PrimeSieve::getStartNumber()   const { return startNumber_; }
uint64_t PrimeSieve::getStopNumber()    const { return stopNumber_; }
uint32_t PrimeSieve::getSieveSize()     const { return sieveSize_; }
uint32_t PrimeSieve::getPreSieveLimit() const { return preSieveLimit_; }

/**
 * Set a start number for sieving.
 * @pre startNumber < (2^64-1) - (2^32-1) * 10
 */
void PrimeSieve::setStartNumber(uint64_t startNumber) {
  // Erat(Medium|Big) stopNumber limit
  if (startNumber >= UINT64_MAX - UINT32_MAX * UINT64_C(10))
    throw std::invalid_argument("START must be < (2^64-1) - (2^32-1) * 10");
  startNumber_ = startNumber;
}

/**
 * Set a stop number for sieving.
 * @pre stopNumber < (2^64-1) - (2^32-1) * 10
 */
void PrimeSieve::setStopNumber(uint64_t stopNumber) {
  // Erat(Medium|Big) stopNumber limit
  if (stopNumber >= UINT64_MAX - UINT32_MAX * UINT64_C(10))
    throw std::invalid_argument("STOP must be < (2^64-1) - (2^32-1) * 10");
  stopNumber_ = stopNumber;
}

/**
 * Set the size of the sieve of Eratosthenes array (in kilobytes).
 * Default sieveSize = 32 KB, the best performance is achieved with a
 * sieve size of the CPU's L1 cache size (usually 32 or 64 KB) when
 * sieving < 10^14 and a sieve size of the CPU's L2 cache size
 * (e.g. 512 KB) above.
 * @param sieveSize  >= 1 && <= 2048 kilobytes, sieveSize is rounded
 *                   up to the next highest power of 2.
 */
void PrimeSieve::setSieveSize(uint32_t sieveSize) {
  // SieveOfEratosthenes needs sieveSize >= 1 kilobyte
  if (sieveSize < 1)
    sieveSize = 1;
  // EratMedium needs sieveSize <= 2048 kilobytes
  if (sieveSize > 2048)
    sieveSize = 2048;
  // EratBig needs a power of 2 sieveSize
  sieveSize_ = nextHighestPowerOf2(sieveSize);
}

/**
 * Multiples of small primes <= preSieveLimit are pre-sieved
 * to speed up the sieve of Eratosthenes.
 * @pre preSieveLimit >= 13 && <= 23
 */
void PrimeSieve::setPreSieveLimit(uint32_t preSieveLimit) {
  // minimum preSieveLimit = 13 (uses 1001 bytes)
  if (preSieveLimit < 13)
    preSieveLimit = 13;
  // maximum preSieveLimit = 23 (uses 7 megabytes)
  if (preSieveLimit > 23)
    preSieveLimit = 23;
  preSieveLimit_ = preSieveLimit;
}

/** Get the current set public flags. */
uint32_t PrimeSieve::getFlags() const {
  return flags_ & ((1U << 20) - 1);
}

bool PrimeSieve::testFlags(uint32_t flags) const {
  return (flags_ & flags) != 0; 
}

/**
 * Settings for sieve().
 * @see   primesieve/docs/USAGE_EXAMPLES
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

/**
 * Get the count of prime numbers within the interval
 * [startNumber, stopNumber].
 */
uint64_t PrimeSieve::getPrimeCount(uint64_t startNumber, 
                                   uint64_t stopNumber) {
  this->setStartNumber(startNumber);
  this->setStopNumber(stopNumber);
  this->setFlags(COUNT_PRIMES);
  this->sieve();
  return this->getPrimeCount();
}

/** Get the count of primes and prime k-tuplets after sieve(). */
uint64_t PrimeSieve::getPrimeCount()      const { return counts_[0]; }
uint64_t PrimeSieve::getTwinCount()       const { return counts_[1]; }
uint64_t PrimeSieve::getTripletCount()    const { return counts_[2]; }
uint64_t PrimeSieve::getQuadrupletCount() const { return counts_[3]; }
uint64_t PrimeSieve::getQuintupletCount() const { return counts_[4]; }
uint64_t PrimeSieve::getSextupletCount()  const { return counts_[5]; }
uint64_t PrimeSieve::getSeptupletCount()  const { return counts_[6]; }

/**
 * Get the count of primes or prime k-tuplets after sieve().
 * @param index = 0 : Count of prime numbers,
 *        index = 1 : Count of twin primes,    
 *        index = 2 : Count of prime triplets,    
 *        index = 3 : Count of prime quadruplets, 
 *        index = 4 : Count of prime quintuplets, 
 *        index = 5 : Count of prime sextuplets,
 *        index = 6 : Count of prime septuplets.
 */
uint64_t PrimeSieve::getCounts(uint32_t index) const {
  if (index >= COUNTS_SIZE)
    throw std::out_of_range("getCounts(uint32_t) index out of range");
  return counts_[index];
}

/** Get the time elapsed in seconds of sieve(). */
double PrimeSieve::getTimeElapsed() const {
  return timeElapsed_;
}

/**
 * Prime number generation methods (32-bit & 64-bit), see
 * primesieve/docs/USAGE_EXAMPLES for usage examples.
 */

void PrimeSieve::generatePrimes(uint32_t startNumber, 
                                uint32_t stopNumber,
                                void (*callback)(uint32_t)) {
  if (callback == NULL)
    throw std::invalid_argument("callback must not be NULL");
  this->setStartNumber(startNumber);
  this->setStopNumber(stopNumber);
  callback32_ = callback;
  flags_      = CALLBACK32_PRIMES;
  this->sieve();
}

void PrimeSieve::generatePrimes(uint32_t startNumber, 
                                uint32_t stopNumber,
                                void (*callback)(uint32_t, void*), void* cbObj) {
  if (callback == NULL || cbObj == NULL)
    throw std::invalid_argument("callback & cbObj must not be NULL");
  this->setStartNumber(startNumber);
  this->setStopNumber(stopNumber);
  callback32_OOP_ = callback;
  cbObj_ = cbObj;
  flags_ = CALLBACK32_OOP_PRIMES;
  this->sieve();
}

void PrimeSieve::generatePrimes(uint64_t startNumber, 
                                uint64_t stopNumber,
                                void (*callback)(uint64_t)) {
  if (callback == NULL)
    throw std::invalid_argument("callback must not be NULL");
  this->setStartNumber(startNumber);
  this->setStopNumber(stopNumber);
  callback64_ = callback;
  flags_      = CALLBACK64_PRIMES;
  this->sieve();
}

void PrimeSieve::generatePrimes(uint64_t startNumber, 
                                uint64_t stopNumber,
                                void (*callback)(uint64_t, void*), void* cbObj) {
  if (callback == NULL || cbObj == NULL)
    throw std::invalid_argument("callback & cbObj must not be NULL");
  this->setStartNumber(startNumber);
  this->setStopNumber(stopNumber);
  callback64_OOP_ = callback;
  cbObj_ = cbObj;
  flags_ = CALLBACK64_OOP_PRIMES;
  this->sieve();
}

void PrimeSieve::reset() {
  segments_ = 0;
  for (int i = 0; i < COUNTS_SIZE; i++)
    counts_[i] = 0;
  status_ = -1.0;
  timeElapsed_ = 0.0;
  parent_->doStatus(0);
}

/**
 * Calculate the current status in percent of sieve().
 * @param processed  The size of the processed segment (interval)
 */
void PrimeSieve::doStatus(uint32_t processed) {
  segments_ += processed;
  double todo = static_cast<double> (stopNumber_ - startNumber_ + 1);
  double done = static_cast<double> (segments_);
  int    old  = static_cast<int> (status_);
  status_ = std::min<double>((done / todo) * 100.0, 100.0);
  if (testFlags(PRINT_STATUS)) {
    int status = static_cast<int> (status_);
    if (status > old)
      std::cout << '\r' << status << '%' << std::flush;
  }
}

void PrimeSieve::doSmallPrime(uint32_t min,
                              uint32_t max,
                              uint32_t type, 
                              const std::string& primeStr)
{
#if defined(_OPENMP)
  #pragma omp critical (generate)
#endif
  if (startNumber_ <= min && stopNumber_ >= max) {
    if (testFlags(CALLBACK_FLAGS) && type == 0) {
      uint32_t prime = primeStr[0] - '0';
      if (testFlags(CALLBACK32_PRIMES))     this->callback32_(prime);
      if (testFlags(CALLBACK32_OOP_PRIMES)) this->callback32_OOP_(prime, cbObj_);
      if (testFlags(CALLBACK64_PRIMES))     this->callback64_(prime);
      if (testFlags(CALLBACK64_OOP_PRIMES)) this->callback64_OOP_(prime, cbObj_);
    } else {
      if (testFlags(COUNT_PRIMES << type)) counts_[type]++;
      if (testFlags(PRINT_PRIMES << type)) std::cout << primeStr << '\n';
    }
  }
}

/**
 * Sieve the primes and prime k-tuplets (twins, triplets, ...) within
 * the interval [startNumber, stopNumber] using a fast segmented sieve
 * of Eratosthenes implementation.
 */
void PrimeSieve::sieve() {
  clock_t t1 = std::clock();
  this->reset();
  if (stopNumber_ < startNumber_)
    throw std::invalid_argument("STOP must be >= START");

  // do small primes and k-tuplets manually
  if (startNumber_ <= 5) {
    this->doSmallPrime(2,  2, 0, "2");
    this->doSmallPrime(3,  3, 0, "3");
    this->doSmallPrime(5,  5, 0, "5");
    this->doSmallPrime(3,  5, 1, "(3, 5)");
    this->doSmallPrime(5,  7, 1, "(5, 7)");
    this->doSmallPrime(5, 11, 2, "(5, 7, 11)");
    this->doSmallPrime(5, 13, 3, "(5, 7, 11, 13)");
    this->doSmallPrime(5, 17, 4, "(5, 7, 11, 13, 17)");
  }

  if (stopNumber_ >= 7) {
    // fast segmented sieve of Eratosthenes object that sieves the
    // primes within [startNumber_, stopNumber_]
    PrimeNumberFinder finder(*this);

    if (finder.needGenerator()) {
      // fast segmented sieve of Eratosthenes object that generates the
      // primes up to sqrt(stopNumber_) needed for sieving by the
      // PrimeNumberFinder
      PrimeNumberGenerator generator(finder);

      // sieve of Eratosthenes implementation that generates the primes
      // up to stopNumber_^0.25 for the PrimeNumberGenerator
      uint32_t N = generator.getSquareRoot();
      std::vector<uint32_t> isPrime(N / 32 + 1, 0xAAAAAAAAu);
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

  // make sure that status_ = 100.0 percent
  parent_->doStatus(10);
  timeElapsed_ = static_cast<double> (std::clock() - t1) / CLOCKS_PER_SEC;
}
