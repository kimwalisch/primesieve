//
// Copyright (c) 2011 Kim Walisch, <kim.walisch@gmail.com>.
// All rights reserved.
//
// This file is part of primesieve.
// Visit: http://primesieve.googlecode.com
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
#include "imath.h"
#include "PreSieve.h"

#include <stdexcept>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>

PrimeSieve::PrimeSieve() :
  startNumber_(0), stopNumber_(0), flags_(COUNT_PRIMES) {
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
PrimeSieve::PrimeSieve(uint64_t startNumber, uint64_t stopNumber, 
    ParallelPrimeSieve* parent) :
  sieveSize_(parent->sieveSize_), 
    flags_(parent->flags_),
    preSieveLimit_(parent->preSieveLimit_),
    callback_(parent->callback_),
    callbackOOP_(parent->callbackOOP_), 
    cbObj_(parent->cbObj_),
    parent_(parent) {
  this->setStartNumber(startNumber);
  this->setStopNumber(stopNumber);
  this->reset();
}

uint64_t PrimeSieve::getStartNumber() const {
  return startNumber_;
}

uint64_t PrimeSieve::getStopNumber() const {
  return stopNumber_;
}

/**
 * Get the sieve size in Kilobytes.
 */
uint32_t PrimeSieve::getSieveSize() const {
  return sieveSize_;
}

uint32_t PrimeSieve::getPreSieveLimit() const {
  return preSieveLimit_;
}

/**
 * Get the current set public flags.
 */
uint32_t PrimeSieve::getFlags() const {
  // clear out private flags
  return flags_ & ((1U << 20) - 1);
}

/**
 * Get the count of prime numbers within the interval
 * [startNumber, stopNumber].
 */
uint64_t PrimeSieve::getPrimeCount(uint64_t startNumber, uint64_t stopNumber) {
  this->setStartNumber(startNumber);
  this->setStopNumber(stopNumber);
  this->setFlags(COUNT_PRIMES);
  this->sieve();
  return this->getPrimeCount();
}

/**
 * Get the count of prime numbers after sieve().
 */
uint64_t PrimeSieve::getPrimeCount() const {
  return counts_[0];
}

/**
 * Get the count of twin primes after sieve().
 */
uint64_t PrimeSieve::getTwinCount() const {
  return counts_[1];
}

/**
 * Get the count of prime triplets after sieve().
 */
uint64_t PrimeSieve::getTripletCount() const {
  return counts_[2];
}

/**
 * Get the count of prime quadruplets after sieve().
 */
uint64_t PrimeSieve::getQuadrupletCount() const {
  return counts_[3];
}

/**
 * Get the count of prime quintuplets after sieve().
 */
uint64_t PrimeSieve::getQuintupletCount() const {
  return counts_[4];
}

/**
 * Get the count of prime sextuplets after sieve().
 */
uint64_t PrimeSieve::getSextupletCount() const {
  return counts_[5];
}

/**
 * Get the count of prime septuplets after sieve().
 */
uint64_t PrimeSieve::getSeptupletCount() const {
  return counts_[6];
}

/**
 * Get the count of prime numbers or prime k-tuplets after sieve().
 * @param type = 0 : Count of prime numbers,
 *        type = 1 : Count of twin primes,    
 *        type = 2 : Count of prime triplets,    
 *        type = 3 : Count of prime quadruplets, 
 *        type = 4 : Count of prime quintuplets, 
 *        type = 5 : Count of prime sextuplets,
 *        type = 6 : Count of prime septuplets.
 */
uint64_t PrimeSieve::getCounts(uint32_t type) const {
  if (type >= COUNTS_SIZE)
    throw std::out_of_range("getCounts(uint32_t) type out of range");
  return counts_[type];
}

/**
 * Get the time elapsed in seconds of sieve().
 */
double PrimeSieve::getTimeElapsed() const {
  return timeElapsed_;
}

/**
 * Set a start number for sieving.
 * @pre startNumber < (2^64-1) - (2^32-1) * 10
 */
void PrimeSieve::setStartNumber(uint64_t startNumber) {
  // EratMedium and EratBig stopNumber limit
  if (startNumber >= UINT64_MAX - UINT32_MAX * UINT64_C(10))
    throw std::invalid_argument("START must be < (2^64-1) - (2^32-1) * 10");
  startNumber_ = startNumber;
}

/**
 * Set a stop number for sieving.
 * @pre stopNumber < (2^64-1) - (2^32-1) * 10
 */
void PrimeSieve::setStopNumber(uint64_t stopNumber) {
  // EratMedium and EratBig stopNumber limit
  if (stopNumber >= UINT64_MAX - UINT32_MAX * UINT64_C(10))
    throw std::invalid_argument("STOP must be < (2^64-1) - (2^32-1) * 10");
  stopNumber_ = stopNumber;
}

/**
 * Set the size of the sieve of Eratosthenes array (in KiloBytes).
 * Default sieveSize = 64 KB.
 * The best performance is achieved with a sieve size that matches
 * the CPU's L1 cache size (usually 32 or 64 KB) when sieving < 10^14
 * and a sieve size of the CPU's L2 cache size (e.g. 512 KB) above.
 *
 * @pre    sieveSize >= 1 && <= 8192 KiloBytes.
 * @remark sieveSize is rounded up to the next highest power of 2
 */
void PrimeSieve::setSieveSize(uint32_t sieveSize) {
  // SieveOfEratosthenes needs sieveSize >= 1 KB
  // EratSmall, EratMedium and EratBig need sieveSize <= 8192
  if (sieveSize < 1 || sieveSize > 8192)
    throw std::invalid_argument("sieve size must be >= 1 && <= 8192 KiloBytes");
  // EratBig needs a power of 2 sieveSize
  sieveSize_ = nextHighestPowerOf2(sieveSize);
}

/**
 * Multiples of small primes <= preSieveLimit are pre-sieved to speed
 * up the sieve of Eratosthenes.
 * @pre preSieveLimit >= 11 && preSieveLimit <= 23
 */
void PrimeSieve::setPreSieveLimit(uint32_t preSieveLimit) {
  if (preSieveLimit < 11 || preSieveLimit > 23)
    throw std::invalid_argument("pre-sieve limit must be >= 11 && <= 23");
  preSieveLimit_ = preSieveLimit;
}

/**
 * Settings for sieve().
 * @see   ../docs/USAGE_EXAMPLES
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

/**
 * Generate the prime numbers within the interval
 * [startNumber, stopNumber] and call a callback function for each
 * prime.
 */
void PrimeSieve::generatePrimes(uint64_t startNumber, uint64_t stopNumber,
    void (*callback)(uint64_t)) {
  if (callback == NULL)
    throw std::invalid_argument("callback must not be NULL");
  this->setStartNumber(startNumber);
  this->setStopNumber(stopNumber);
  flags_ = CALLBACK_PRIMES;
  callback_ = callback;
  this->sieve();
}

/**
 * Generate the prime numbers within the interval
 * [startNumber, stopNumber] and call an OOP callback function for
 * each prime.
 */
void PrimeSieve::generatePrimes(uint64_t startNumber, uint64_t stopNumber,
    void (*callback)(uint64_t, void*), void* cbObj) {
  if (callback == NULL)
    throw std::invalid_argument("callback must not be NULL");
  this->setStartNumber(startNumber);
  this->setStopNumber(stopNumber);
  flags_ = CALLBACK_PRIMES_OOP;
  callbackOOP_ = callback;
  cbObj_ = cbObj;
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
 * @param processed A sieved interval (segment)
 */
void PrimeSieve::doStatus(uint32_t processed) {
  segments_ += processed;
  int old = static_cast<int> (status_);
  status_ = std::min<double>(100.0, (static_cast<double> (segments_) / 
      (1 + stopNumber_ - startNumber_)) * 100.0);
  if (flags_ & PRINT_STATUS) {
    int status = static_cast<int> (status_);
    if (status > old)
      std::cout << '\r' << status << '%' << std::flush;
  }
}

void PrimeSieve::doSmallPrime(uint32_t low, uint32_t high, uint32_t type, 
    const std::string& prime) {
  if (startNumber_ <= low && stopNumber_ >= high) {
    if (flags_ & (COUNT_PRIMES << type))
      counts_[type]++;
    if (flags_ & (PRINT_PRIMES << type))
      std::cout << prime << std::endl;
    else if (type == 0 && (flags_ & CALLBACK_PRIMES))
      this->callback_(prime[0]-'0');
    else if (type == 0 && (flags_ & CALLBACK_PRIMES_OOP))
      this->callbackOOP_(prime[0]-'0', cbObj_);
  }
}

/**
 * Sieve the prime numbers and/or prime k-tuplets within the interval
 * [startNumber_, stopNumber_].
 */
void PrimeSieve::sieve() {
  clock_t t1 = std::clock();
  this->reset();
  if (stopNumber_ < startNumber_)
    throw std::invalid_argument("STOP must be >= START");

  // small primes have to be examined manually
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
    // used to sieve the primes and prime k-tuplets within the
    // interval [startNumber_, stopNumber_]
    PrimeNumberFinder finder(*this);

    if (isqrt(stopNumber_) > finder.getPreSieveLimit()) {
      /// used to generate the primes up to stopNumber_^0.5 needed for
      /// sieving by finder
      /// @see PrimeNumberGenerator::generate(const uint8_t*, uint32_t)
      PrimeNumberGenerator generator(finder);

      // the following sieve of Eratosthenes implementation generates
      // the primes up to stopNumber_^0.25 needed for sieving by the
      // faster PrimeNumberGenerator
      uint32_t N = isqrt(generator.getStopNumber());
      std::vector<uint8_t> isPrime(N/8+1, 0xAA);
      for (uint32_t i = 3; i*i <= N; i += 2) {
        if (isPrime[i>>3] & (1<<(i&7)))
          for (uint32_t j = i*i; j <= N; j += i*2)
            isPrime[j>>3] &= ~(1<<(j&7));
      }
      for (uint32_t i = generator.getPreSieveLimit() + 1; i <= N; i++) {
        if (isPrime[i>>3] & (1<<(i&7)))
          generator.sieve(i);
      }
      generator.finish();
    }
    finder.finish();
  }
  // set status_ to 100.0 percent
  parent_->doStatus(10);
  timeElapsed_ = static_cast<double> (std::clock() - t1) / CLOCKS_PER_SEC;
}
