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
 * EratMedium and EratBig use WheelPrimes to cross off multiples. Each
 * WheelPrime object contains the sieving prime, the position of the
 * next multiple within the SieveOfEratosthenes array (sieve index)
 * and a wheel index.
 *
 * @remark WheelPrime_1  Uses 8 bytes per sieving prime, WheelPrime_1
 *                       is used in EratSmall and EratBig.
 */
class WheelPrime_1 {
public:
  uint32_t getSievingPrime() const {
    return sievingPrime_;
  }
  uint32_t getSieveIndex() const {
    // get the 23 least significant bits
    return indexes_ & ((1U << 23) - 1);
  }
  uint32_t getWheelIndex() const {
    // get the 9 most significant bits
    return indexes_ >> 23;
  }
  void set(uint32_t sievingPrime,
           uint32_t sieveIndex,
           uint32_t wheelIndex)
  {
    assert(sieveIndex < (1U << 23) &&
           wheelIndex < (1U << 9));
    indexes_      = sieveIndex | (wheelIndex << 23);
    sievingPrime_ = sievingPrime;
  }
  void setWheelIndex(uint32_t wheelIndex) {
    assert(wheelIndex < (1U << 9));
    indexes_ = wheelIndex << 23;
  }
  void setSieveIndex(uint32_t sieveIndex) {
    assert(sieveIndex < (1U << 23));
    indexes_ |= sieveIndex;
  }
private:
  /**
   * sieveIndex = 23 least significant bits of indexes_.
   * wheelIndex =  9 most  significant bits of indexes_.
   * Packing sieveIndex and wheelIndex into the same 32-bit word
   * reduces primesieve's memory usage by 20%.
   */
  uint32_t indexes_;
  /**
   * sievingPrime_ = prime / 15;
   * /15 = *2 /30, *2 is used to skip multiples of 2, /30 is used as
   * SieveOfEratosthenes objects use 30 numbers per byte.
   * @see ModuloWheel::getWheelPrimeData()
   */
  uint32_t sievingPrime_;
};

/**
 * @see    WheelPrime_1
 * @remark WheelPrime_2  Unlike WheelPrime_1 WheelPrime_2 allows
 *                       sieveIndex   >  23-bits but requires
 *                       sievingPrime <= 23-bits.
 */
class WheelPrime_2 {
public:
  uint32_t getSievingPrime() const {
    // get the 23 most significant bits
    return data_ >> 9;
  }
  uint32_t getSieveIndex() const {
    return sieveIndex;
  }
  uint32_t getWheelIndex() const {
    // get the 9 least significant bits
    return data_ & ((1U << 9) - 1);
  }
  void set(uint32_t sievingPrime,
           uint32_t sieveIndex,
           uint32_t wheelIndex)
  {
    assert(sievingPrime < (1U << 23) &&
           wheelIndex   < (1U << 9));
    uint32_t packed  = wheelIndex | (sievingPrime << 9);
    this->sieveIndex = sieveIndex;
    data_            = packed;
  }
  uint32_t sieveIndex;
private:
  /**
   * wheelIndex   =  9 least significant bits of data_.
   * sievingPrime = 23 most  significant bits of data_.
   * Packing wheelIndex and sievingPrime into the same 32-bit word
   * reduces primesieve's memory usage by 20%.
   */
  uint32_t data_;
};

/**
 * A Bucket is a container for sieving primes <= sqrt(n), it is
 * designed as a singly linked list, once there is no more space in
 * the current bucket a new bucket node is created ...
 * The Bucket data structure is due to Tomas Oliveira, see
 * http://www.ieeta.pt/~tos/software/prime_sieve.html
 *
 * @param T_WheelPrime  A WheelPrime object is a sieving prime for use
 *                      with wheel factorization.
 */
template<class T_WheelPrime>
class Bucket {
public:
  Bucket() : count_(0) {
  }
  // list::push_back(Bucket()) adds an empty bucket without
  // unnecessary copying
  Bucket(const Bucket&) : count_(0) {
  }
  void reset() {
    count_ = 0;
  }
  uint32_t getCount() const {
    return count_;
  }
  Bucket* next() {
    return next_;
  }
  T_WheelPrime* getWheelPrimes() {
    return wheelPrimes_;
  }
  void setNext(Bucket* next) {
    next_ = next;
  }
  bool isEmpty() const {
    return (count_ == 0);
  }
  bool hasNext() const {
    return (next_ != NULL);
  }
  /**
   * Add a WheelPrime to the Bucket.
   * @return false  If the bucket is full else true.
   */
  bool addWheelPrime(uint32_t sievingPrime,
                     uint32_t sieveIndex,
                     uint32_t wheelIndex) {
    uint32_t pos = count_;
    count_ += 1;
    assert(pos < defs::BUCKET_SIZE);
    wheelPrimes_[pos].set(sievingPrime, sieveIndex, wheelIndex);
    return (pos != defs::BUCKET_SIZE - 1);
  }
private:
  uint32_t count_;
  Bucket* next_;
  T_WheelPrime wheelPrimes_[defs::BUCKET_SIZE];
};

/**
 * In EratMedium and EratBig the wheel is implemented using a static
 * const array of WheelElement objects. The WheelElement data
 * structure helps to cross-off (unset bit) the current multiple of a
 * sieving prime and to calculate the prime's next multiple.
 * @see EratMedium::sieve()
 */
struct WheelElement {
  /**
   * Bitmask used with the '&' operator to unset the bit corresponding
   * to the current multiple of a sieving prime (WheelPrime).
   */
  uint8_t unsetBit;
  /**
   * Factor used to calculate the next multiple of a sieving prime
   * (WheelPrime) that is not a multiple of any of the wheel's factors
   * (e.g. not a multiple of 2, 3, and 5 for a modulo 30 wheel).
   */
  uint8_t nextMultipleFactor;
  /**
   * Overflow needed to correct the next sieve index:
   * sieveIndex_ = sievingPrime_ * nextMultipleFactor + correct
   */
  uint8_t correct;
  /** 
   * Used to calculate the next wheel index:
   * wheelIndex_ = wheelIndex_ + next
   */
   int8_t next;
};

struct InitWheel {
  uint8_t nextMultipleFactor;
  uint8_t wheelIndex;
};

/**
 * Used to initialize sieving primes for use with the Modulo30Wheel
 * class. This lookup table is used to calculate the first
 * multiple >= startNumber that is not divisible by 2, 3 and 5 of
 * each sieving prime (and the related wheel index).
 */
extern const InitWheel init30Wheel[30];
/**
 * Used to initialize sieving primes for use with the Modulo210Wheel
 * class. This lookup table is used to calculate the first
 * multiple >= startNumber that is not divisible by 2, 3, 5 and 7 of
 * each sieving prime (and the related wheel index).
 */
extern const InitWheel init210Wheel[210];

/**
 * Abstract class that is used to initialize sieving primes <= sqrt(n)
 * for use with wheel factorization.
 */
template<uint32_t WHEEL_MODULO, uint32_t WHEEL_ELEMENTS,
    const InitWheel* INIT_WHEEL>
class ModuloWheel {
private:
  static const uint32_t primeTypes_[15];
  /** Reference to the parent SieveOfEratosthenes object. */
  const SieveOfEratosthenes& soe_;
  /** Uncopyable, declared but not defined. */
  ModuloWheel(const ModuloWheel&);
  ModuloWheel& operator=(const ModuloWheel&);
protected:
  ModuloWheel(const SieveOfEratosthenes& soe) : 
    soe_(soe) {
    // prevent 64-bit overflows of multiple in getWheelPrimeData()
    uint64_t maxSievingPrime = UINT32_MAX;
    uint64_t maxInitFactor   = INIT_WHEEL[2].nextMultipleFactor + 1;
    uint64_t limit           = UINT64_MAX - maxSievingPrime * maxInitFactor;
    if (soe_.getStopNumber() > limit) {
      std::ostringstream error;
      error << "ModuloWheel: stopNumber must be <= (2^64-1) - (2^32-1) * "
            << maxInitFactor
            << ".";
      throw std::overflow_error(error.str());
    }
    // max sieveSize = max WheelPrime::getSieveIndex() + 1 = 2^23
    // also sieveSize <= 2^28 in order to prevent 32-bit overflows of
    // sieveIndex in Erat*::sieve()
    if (soe_.getSieveSize() > (1U << 23))
      throw std::overflow_error(
          "ModuloWheel: sieveSize must be <= 2^23, 8192 kilobytes.");
  }
  ~ModuloWheel() {}
  /**
   * Used to initialize sieving primes <= sqrt(n) for use with wheel
   * factorization. Calculates the first multiple >= startNumber of
   * prime that is not divisible by any of the wheel's prime factors
   * (e.g. 2, 3 and 5 for a modulo 30 wheel) and the position within
   * the SieveOfEratosthenes array (sieveIndex) of that multiple and
   * its wheel index.
   * @return true if the WheelPrime must be stored for sieving else
   *         false.
   */
  bool getWheelPrimeData(uint32_t* prime,
                         uint32_t* sieveIndex,
                         uint32_t* wheelIndex) const
  {
    uint64_t segmentLow = soe_.getSegmentLow();
    assert(segmentLow % 30 == 0);
    // '+ 6' is a correction for primes of type i*30 + 31
    segmentLow += 6;
    // calculate the first multiple > segmentLow of prime
    uint64_t quotient = segmentLow / *prime + 1;
    uint64_t multiple = *prime * quotient;
    // prime not needed for sieving
    if (multiple > soe_.getStopNumber())
      return false;
    // by theory prime^2 is the first multiple of prime
    // that needs to be crossed-off
    uint64_t primeSquared = isquare<uint64_t>(*prime);
    if (primeSquared > multiple) {
       multiple = primeSquared;
       quotient = *prime;
    }
    uint32_t index = static_cast<uint32_t> (quotient % WHEEL_MODULO);
    // calculate the next multiple that is not divisible by any of the
    // wheel's primes (e.g. 2, 3 and 5 for a modulo 30 wheel)
    multiple += static_cast<uint64_t> (*prime) * INIT_WHEEL[index].nextMultipleFactor;
    if (multiple > soe_.getStopNumber())
      return false;
    *sieveIndex = static_cast<uint32_t> ((multiple - segmentLow) / 30);
    uint32_t wheelOffset = WHEEL_ELEMENTS * primeTypes_[*prime % 15];
    *prime /= 15;
    *wheelIndex = wheelOffset + INIT_WHEEL[index].wheelIndex;
    return true;
  }
};

/**
 * 8 different types of primes, one type for each bit of a byte.
 * prime type = primeTypes_[prime % 15];
 * 0xff values are never accessed.
 */
template<uint32_t WHEEL_MODULO, uint32_t WHEEL_ELEMENTS,
    const InitWheel* INIT_WHEEL>
const uint32_t
    ModuloWheel<WHEEL_MODULO, WHEEL_ELEMENTS, INIT_WHEEL>::primeTypes_[15] = {
        0xff, 7, 3, 0xff, 4, 0xff, 0xff, 0, 5, 0xff, 0xff, 1, 0xff, 2, 6 };

/**
 * Modulo 30 wheel (3rd wheel) implementation.
 * EratSmall is derived from Modulo30Wheel.
 */
class Modulo30Wheel: protected ModuloWheel<30, 8, init30Wheel> {
protected:
  static const WheelElement wheel_[8 * 8];
  Modulo30Wheel(const SieveOfEratosthenes& soe) :
    ModuloWheel<30, 8, init30Wheel> (soe) {
  }
  ~Modulo30Wheel() {}
};

/**
 * Modulo 210 wheel (4th wheel) implementation.
 * EratMedium and EratBig are derived from Modulo210Wheel and use its
 * wheel_ array to skip multiples of 2, 3, 5 and 7.
 */
class Modulo210Wheel: protected ModuloWheel<210, 48, init210Wheel> {
protected:
  static const WheelElement wheel_[48 * 8];
  Modulo210Wheel(const SieveOfEratosthenes& soe) :
    ModuloWheel<210, 48, init210Wheel> (soe) {
  }
  ~Modulo210Wheel() {}
};

#endif /* WHEELFACTORIZATION_H */
