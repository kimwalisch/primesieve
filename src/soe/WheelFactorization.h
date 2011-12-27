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

/** 
 * @file WheelFactorization.h
 * @brief Contains classes and structs related to wheel factorization.
 *
 * Wheel factorization is used to skip multiples of small primes to
 * speed up the sieve of Eratosthenes.
 * http://en.wikipedia.org/wiki/Wheel_factorization
 */

#ifndef WHEELFACTORIZATION_H
#define WHEELFACTORIZATION_H

#include "SieveOfEratosthenes.h"
#include "defs.h"
#include "imath.h"

#include <cassert>
#include <stdexcept>
#include <sstream>

/**
 * WheelPrime objects are sieving primes <= sqrt(n) for use with wheel
 * factorization (skips multiples of small primes). EratSmall,
 * EratMedium and EratBig use WheelPrimes to cross-off multiples. Each
 * WheelPrime object contains the sieving prime, the position of the
 * next multiple within the SieveOfEratosthenes array (multipleIndex)
 * and a wheel index.
 * @remark WheelPrime  Uses 8 bytes per sieving prime.
 */
class WheelPrime {
public:
  uint32_t getSievingPrime() const {
    return sievingPrime_;
  }
  uint32_t getMultipleIndex() const {
    // get the 23 least significant bits
    return indexes_ & ((1U << 23) - 1);
  }
  uint32_t getWheelIndex() const {
    // get the 9 most significant bits
    return indexes_ >> 23;
  }
  void set(uint32_t sievingPrime,
           uint32_t multipleIndex,
           uint32_t wheelIndex) {
    assert(multipleIndex < (1U << 23));
    assert(wheelIndex    < (1U << 9));
    indexes_ = multipleIndex | (wheelIndex << 23);
    sievingPrime_ = sievingPrime;
  }
  void setIndexes(uint32_t multipleIndex,
                  uint32_t wheelIndex) {
    assert(multipleIndex < (1U << 23));
    assert(wheelIndex    < (1U << 9));
    indexes_ = multipleIndex | (wheelIndex << 23);
  }
  void setWheelIndex(uint32_t wheelIndex) {
    assert(wheelIndex < (1U << 9));
    indexes_ = wheelIndex << 23;
  }
  void setMultipleIndex(uint32_t multipleIndex) {
    assert(multipleIndex < (1U << 23));
    indexes_ |= multipleIndex;
  }
private:
  /**
   * multipleIndex = 23 least significant bits of indexes_.
   * wheelIndex    =  9 most  significant bits of indexes_.
   * Packing multipleIndex and wheelIndex into the same 32-bit word
   * reduces primesieve's memory usage by 20%.
   */
  uint32_t indexes_;
  /**
   * sievingPrime_ = prime / 30;
   * '/ 30' is used as SieveOfEratosthenes objects use a bit array
   * with 30 numbers per byte.
   * @see Wheel::getWheelPrimeData()
   */
  uint32_t sievingPrime_;
};

/**
 * A Bucket is a container for sieving primes <= sqrt(n), it is
 * designed as a singly linked list, once there is no more space in
 * the current bucket a new bucket node is created ...
 * The Bucket data structure is due to Tomas Oliveira, see
 * http://www.ieeta.pt/~tos/software/prime_sieve.html
 */
class Bucket {
public:
  Bucket() : current_(wheelPrimes_) { }
  // list::push_back(Bucket()) adds an empty bucket
  // without unnecessary copying
  Bucket(const Bucket&) : current_(wheelPrimes_) { }
  void reset() {
    current_ = wheelPrimes_;
  }
  WheelPrime* begin() {
    return wheelPrimes_;
  }
  WheelPrime* end() {
    return current_;
  }
  Bucket* next() {
    return next_;
  }
  void setNext(Bucket* next) {
    next_ = next;
  }
  bool isEmpty() const {
    return current_ == wheelPrimes_;
  }
  bool hasNext() const {
    return next_ != NULL;
  }
  /**
   * Add a WheelPrime to the Bucket.
   * @return false  If the bucket is full else true.
   */
  bool addWheelPrime(uint32_t sievingPrime,
                     uint32_t multipleIndex,
                     uint32_t wheelIndex)
  {
    WheelPrime* wPrime = current_;
    current_++;
    wPrime->set(sievingPrime, multipleIndex, wheelIndex);
    return (wPrime != &wheelPrimes_[defs::BUCKET_SIZE - 1]);
  }
private:
  WheelPrime* current_;
  Bucket* next_;
  WheelPrime wheelPrimes_[defs::BUCKET_SIZE];
};

/**
 * An array of WheelInit objects is used to calculate the first
 * multiple >= startNumber of each sieving prime that is not
 * divisible by any of the wheel's factors.
 */
struct WheelInit {
  uint8_t nextMultipleFactor;
  uint8_t wheelIndex;
};

/**
 * In EratMedium and EratBig the wheel is implemented using a global
 * const array of WheelElement objects. The WheelElement data
 * structure is used to cross-off (unset bit) the current multiple of
 * sieving primes and to calculate their next multiple.
 * @see EratMedium::sieve()
 */
struct WheelElement {
  WheelElement(uint8_t _unsetBit,
               uint8_t _nextMultipleFactor,
               uint8_t _correct,
                int8_t _next) :
    unsetBit(_unsetBit),
    nextMultipleFactor(_nextMultipleFactor),
    correct(_correct),
    next(_next) { }
  /**
   * Bitmask used with the '&' operator to unset the bit (within the
   * SieveOfEratosthenes array) corresponding to the current multiple
   * of a WheelPrime object.
   */
  uint8_t unsetBit;
  /**
   * Factor used to calculate the next multiple of a WheelPrime object
   * (sieving prime) that is coprime to the wheel's factors
   * (e.g. not a multiple of 2, 3, and 5 for a modulo 30 wheel).
   */
  uint8_t nextMultipleFactor;
  /**
   * Overflow needed to correct the next multiple offset i.e.
   * multipleIndex += prime * nextMultipleFactor + correct;
   */
  uint8_t correct;
  /**
   * Offset that is used to calculate the next wheel index of a
   * WheelPrime object i.e. wheelIndex += next;
   */
   int8_t next;
};

/**
 * The Wheel class uses wheel factorization to skip multiples of small
 * primes in the sieve of Eratosthenes. Via template arguments it is
 * possible to build different Wheel classes e.g. Modulo30Wheel_t and
 * Modulo210Wheel_t. The Erat(Small|Medium|Big) classes are derived
 * from Wheel.
 */
template<uint32_t            WHEEL_MODULO,
         uint32_t            WHEEL_SIZE,
         const WheelElement* WHEEL_ARRAY,
         const WheelInit*    WHEEL_INIT>
class Wheel {
private:
  static const uint32_t wheelOffsets_[30];
  /** Reference to the parent SieveOfEratosthenes object. */
  const SieveOfEratosthenes& soe_;
  Wheel(const Wheel&);
  Wheel& operator=(const Wheel&);
protected:
  Wheel(const SieveOfEratosthenes& soe) : soe_(soe) {
    uint64_t maxSievingPrime = UINT32_MAX;
    uint64_t maxInitFactor   = WHEEL_INIT[2].nextMultipleFactor + 1;
    uint64_t limit           = UINT64_MAX - maxSievingPrime * maxInitFactor;
    // prevent 64-bit overflows of multiple in getWheelPrimeData()
    if (soe_.getStopNumber() > limit) {
      std::ostringstream error;
      error << "Wheel: stopNumber must be <= (2^64-1) - (2^32-1) * "
            << maxInitFactor
            << ".";
      throw std::overflow_error(error.str());
    }
    // max(sieveSize) = max(WheelPrime::getMultipleIndex()) + 1 = 2^23
    if (soe_.getSieveSize() > (1U << 23))
      throw std::overflow_error(
          "Wheel: sieveSize must be <= 2^23, 8192 kilobytes.");
  }
  ~Wheel() { }
  /**
   * Used to initialize sieving primes <= sqrt(n) for use with wheel
   * factorization. Calculates the first multiple >= startNumber of
   * prime that is not divisible by any of the wheel's prime factors
   * (e.g. 2, 3 and 5 for a modulo 30 wheel) and the position within
   * the SieveOfEratosthenes array (multipleIndex) of that multiple
   * and its wheel index.
   * @return true if the WheelPrime must be stored for sieving
   *         else false.
   */
  bool getWheelPrimeData(uint32_t* prime,
                         uint32_t* multipleIndex,
                         uint32_t* wheelIndex) const
  {
    // '+ 6' is a correction for sieving primes of type i*30 + 31
    const uint64_t segmentLow = soe_.getSegmentLow() + 6;
    // calculate the first multiple > segmentLow of prime
    uint64_t quotient = segmentLow / *prime + 1;
    uint64_t multiple = *prime * quotient;
    if (multiple > soe_.getStopNumber())
      return false;
    const uint64_t primeSquared = isquare<uint64_t>(*prime);
    // by theory prime^2 is the first multiple of prime
    // that needs to be crossed-off
    if (multiple < primeSquared) {
      multiple = primeSquared;
      quotient = *prime;
    }
    const WheelInit& wheelInit = WHEEL_INIT[quotient % WHEEL_MODULO];
    // calculate the next multiple that is not divisible by any of the
    // wheel's primes (e.g. 2, 3 and 5 for a modulo 30 wheel)
    multiple += static_cast<uint64_t> (*prime) * wheelInit.nextMultipleFactor;
    if (multiple > soe_.getStopNumber())
      return false;
    *multipleIndex = static_cast<uint32_t> ((multiple - segmentLow) / 30);
    *wheelIndex = wheelOffsets_[*prime % 30] + wheelInit.wheelIndex;
    *prime /= 30;
    return true;
  }
  const WheelElement* wheel(uint32_t n) const {
    assert(n < WHEEL_SIZE * 8);
    return &WHEEL_ARRAY[n];
  }
};

/**
 * The wheelOffsets_ array is used to calculate the position of the
 * first multiple >= startNumber within the WHEEL_ARRAY.
 * There are 8 different types of sieving primes, one type for each
 * bit of a byte thus there are also 8 different offsets.
 */
template<uint32_t            WHEEL_MODULO,
         uint32_t            WHEEL_SIZE,
         const WheelElement* WHEEL_ARRAY,
         const WheelInit*    WHEEL_INIT>
const uint32_t
    Wheel<WHEEL_MODULO, WHEEL_SIZE, WHEEL_ARRAY, WHEEL_INIT>::wheelOffsets_[30] = {
        0xFF, 7 * WHEEL_SIZE, 0xFF, 0xFF, 0xFF,           0xFF,
        0xFF, 0 * WHEEL_SIZE, 0xFF, 0xFF, 0xFF, 1 * WHEEL_SIZE,
        0xFF, 2 * WHEEL_SIZE, 0xFF, 0xFF, 0xFF, 3 * WHEEL_SIZE,
        0xFF, 4 * WHEEL_SIZE, 0xFF, 0xFF, 0xFF, 5 * WHEEL_SIZE,
        0xFF,           0xFF, 0xFF, 0xFF, 0xFF, 6 * WHEEL_SIZE };

/** Wheel arrays defined in WheelFactorization.cpp */
extern const WheelInit wheel30Init[30];
extern const WheelInit wheel210Init[210];
extern const WheelElement wheel30Array[8*8];
extern const WheelElement wheel210Array[48*8];

/** 3rd wheel, skips multiples of 2, 3 and 5. */
typedef Wheel< 30,  8, wheel30Array,  wheel30Init> Modulo30Wheel_t;
/** 4th wheel, skips multiples of 2, 3, 5 and 7. */
typedef Wheel<210, 48, wheel210Array, wheel210Init> Modulo210Wheel_t;

#endif /* WHEELFACTORIZATION_H */
