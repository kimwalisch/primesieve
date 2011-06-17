/*
 * WheelFactorization.h -- This file is part of primesieve
 *
 * Copyright (C) 2011 Kim Walisch, <kim.walisch@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

/** 
 * @file WheelFactorization.h
 * @brief Contains classes and structs related to wheel factorization.
 *
 * Wheel factorization is used to skip multiples of small primes and
 * thus speed up the sieve of Eratosthenes.
 * http://en.wikipedia.org/wiki/Wheel_factorization
 * http://primes.utm.edu/glossary/xpage/WheelFactorization.html
 */

#ifndef WHEELFACTORIZATION_H
#define WHEELFACTORIZATION_H

#include "SieveOfEratosthenes.h"
#include "defs.h"
#include "pmath.h"

#include <stdexcept>
#include <sstream>
#include <cassert>

/**
 * WheelPrime objects are sieving primes <= n^0.5 for use with wheel
 * factorization. EratSmall, EratMedium and EratBig use WheelPrimes to
 * cross off multiples.
 * Each WheelPrime contains the sieving prime (sievingPrime_), the
 * position of the next multiple within the SieveOfEratosthenes array
 * (sieveIndex_) and an index for the wheel (wheelIndex_).
 */
class WheelPrime {
public:
  uint32_t getSievingPrime() const {
    return sievingPrime_;
  }
  uint32_t getSieveIndex() const {
    return index_ & 0x7FFFFF;
  }
  uint32_t getWheelIndex() const {
    return index_ >> 23;
  }
  void setSievingPrime(uint32_t sievingPrime) {
    sievingPrime_ = sievingPrime;
  }
  void setSieveIndex(uint32_t sieveIndex) {
    index_ |= sieveIndex;
  }
  void setWheelIndex(uint32_t wheelIndex) {
    index_ = wheelIndex << 23;
  }
  /**
   * sievingPrime_ = prime / 15;
   * /15 = *2/30, *2 is used to skip multiples of 2 and /30 is used as
   * SieveOfEratosthenes objects use 30 numbers per byte.
   * @see ModuloWheel::setWheelPrime(...)
   */
  uint32_t sievingPrime_;
  /**
   * sieveIndex_ = 23 least significant bits of index_.
   * wheelIndex_ =  9 most significant bits of index_.
   * Packing sieveIndex_ and wheelIndex_ into the same 32 bit word
   * reduces primesieve's memory usage by 20%.
   */
  uint32_t index_;
};

/**
 * A Bucket is a container for WheelPrimes.
 * If the current Bucket is full of WheelPrimes a new empty Bucket is
 * created that sets its next node to the full Bucket. This singly
 * linked list approach allows to dynamically manage memory for
 * WheelPrimes.
 */
template<uint32_t SIZE>
class Bucket {
public:
  Bucket* next;
  Bucket() : count_(0) {}
  void setNext(Bucket* _next) {
    next = _next;
  }
  void reset() {
    count_ = 0;
  }
  /** Pointer to the first WheelPrime within the Bucket. */
  WheelPrime* wheelPrimeBegin() {
    return wheelPrime_;
  }
  WheelPrime* wheelPrimeEnd() {
    return &wheelPrime_[count_];
  }
  /**
   * Add a WheelPrime to the Bucket.
   * @return false if the bucket is full else true.
   */
  bool addWheelPrime(uint32_t sievingPrime,
                     uint32_t sieveIndex,
                     uint32_t wheelIndex)
  {
    uint32_t pos = count_++;
    assert(pos < SIZE);
    WheelPrime& wPrime = wheelPrime_[pos];
    wPrime.setSievingPrime(sievingPrime);
    wPrime.setWheelIndex(wheelIndex);
    wPrime.setSieveIndex(sieveIndex);
    return (pos != SIZE - 1);
  }
private:
  /** Count of WheelPrimes within the Bucket. */
  uint32_t count_;
  WheelPrime wheelPrime_[SIZE];
};

/**
 * In EratMedium and EratBig the wheel is implemented using a constant
 * array of WheelElement objects.
 * In Erat*::sieve() an array of WheelElements is used to cross off
 * the current multiple of WheelPrimes and to calculate their next
 * multiple.
 */
struct WheelElement {
  /**
   * Bitmask used with the '&' operator to unset the bit corresponding
   * to the current multiple of a WheelPrime:
   * SieveOfEratosthenes::sieve_[sieveIndex] &= unsetBit;
   */
  uint8_t unsetBit;
  /**
   * Factor used to calculate the next multiple of the a WheelPrime.
   */
  uint8_t nextMultipleFactor;
  /**
   * Overflow needed to correct the next sieve index of a WheelPrime:
   * sieveIndex_ = sievingPrime_ * nextMultipleFactor + correct
   */
  uint8_t correct;
  /** 
   * Used to calculate the next wheel index of a WheelPrime:
   * wheelIndex_ = wheelIndex_ + next
   */
   int8_t next;
};

struct InitWheel {
  uint8_t nextMultipleFactor;
  uint8_t wheelIndex;
};

/**
 * Used to initialize a sieving prime for use with a modulo 30 wheel.
 * This lookup table is used to calculate the first
 * multiple >= startNumber_ that is not divisible by 2, 3 and 5 of a
 * sieving prime and the wheel index of that multiple.
 */
extern const InitWheel init30Wheel[30];
/**
 * Used to initialize a sieving prime for use with a modulo 210 wheel.
 * This lookup table is used to calculate the first
 * multiple >= startNumber_ that is not divisible by 2, 3, 5 and 7 of
 * a sieving prime and the wheel index of that multiple.
 */
extern const InitWheel init210Wheel[210];

/**
 * Abstract class that is used to initialize sieving primes <= n^0.5
 * for use with wheel factorization.
 */
template<uint32_t WHEEL_MODULO, uint32_t WHEEL_ELEMENTS,
    const InitWheel* INIT_WHEEL>
class ModuloWheel {
private:
  /** Used in the wheel index calculation of sieving primes. */
  static const uint8_t primeBitPosition_[30];
protected:
  const uint64_t stopNumber_;
  ModuloWheel(const SieveOfEratosthenes* soe) : 
    stopNumber_(soe->getStopNumber()) {
    uint64_t greatestWheelFactor = INIT_WHEEL[2].nextMultipleFactor;
    // prevents 64 bit overflows of multiple in setWheelPrime()
    if (stopNumber_ > UINT64_MAX - UINT32_MAX * (greatestWheelFactor + 1)) {
      std::ostringstream error;
      error << "ModuloWheel: stopNumber must be <= (2^64-1) - (2^32-1) * "
          << greatestWheelFactor + 1 << ".";
      throw std::overflow_error(error.str());
    }
    // max sieveSize = max WheelPrime::sieveIndex_ + 1 = 2^23
    // also sieveSize <= 2^28 in order to prevent 32 bit overflows of
    // sieveIndex in Erat*::sieve()
    if (soe->getSieveSize() > (1u << 23))
      throw std::overflow_error(
          "ModuloWheel: sieveSize must be <= 2^23, 8192 Kilobytes.");
  }
  ~ModuloWheel() {
  }
  /**
   * Used to initialize sieving primes <= n^0.5 for use with wheel
   * factorization.
   * Calculates the first multiple >= startNumber_ that is not
   * divisible by any of the wheel's prime factors (i.e. 2, 3, 5 for a
   * modulo 30 wheel) of prime and the position within the
   * SieveOfEratosthenes array (sieveIndex) of that multiple and the
   * wheel index of that multiple.
   * 
   * @remark The terms '+ 6' and '- 6' are corrections needed for
   *         primes type i*30 + 31.
   * @return true if the WheelPrime must be stored for sieving else
   *         false.
   */
  bool setWheelPrime(uint64_t segmentLow, 
                     uint32_t* prime,
                     uint32_t* sieveIndex,
                     uint32_t* wheelIndex)
  {
    assert(segmentLow % 30 == 0);
    // by theory prime^2 is the first multiple of prime that needs to
    // be crossed off
    uint64_t multiple = isquare(*prime);
    uint64_t quotient = *prime;
    // calculate the first multiple > segmentLow + 6 of prime
    if (multiple < segmentLow + 6) {
      quotient = (segmentLow + 6) / *prime + 1;
      multiple = *prime * quotient;
      // prime not needed for sieving
      if (multiple > stopNumber_)
        return false;
    }
    uint32_t index = static_cast<uint32_t> (quotient % WHEEL_MODULO);
    // calculate the next multiple that is not divisible by any of the
    // wheel's primes (i.e. 2, 3 and 5 for a modulo 30 wheel)
    multiple += static_cast<uint64_t> (*prime) * INIT_WHEEL[index].nextMultipleFactor;
    if (multiple > stopNumber_)
      return false;
    uint32_t wheelOffset = WHEEL_ELEMENTS * primeBitPosition_[*prime % 30];
    *wheelIndex = wheelOffset + INIT_WHEEL[index].wheelIndex;
    *sieveIndex = static_cast<uint32_t> (((multiple - segmentLow) - 6) / 30);
    *prime /= 15;
    return true;
  }
};

// 0xff values are never accessed
template<uint32_t WHEEL_MODULO, uint32_t WHEEL_ELEMENTS,
    const InitWheel* INIT_WHEEL>
const uint8_t
    ModuloWheel<WHEEL_MODULO, WHEEL_ELEMENTS, INIT_WHEEL>::primeBitPosition_[30] = { 0xff,
           7, 0xff, 0xff, 0xff, 0xff, 0xff,
           0, 0xff, 0xff, 0xff,    1, 0xff,
           2, 0xff, 0xff, 0xff,    3, 0xff,
           4, 0xff, 0xff, 0xff,    5, 0xff,
        0xff, 0xff, 0xff, 0xff,    6 };

/**
 * Implementation of a modulo 30 wheel (3rd wheel)
 * EratSmall is derived from Modulo30Wheel.
 */
class Modulo30Wheel: protected ModuloWheel<30, 8, init30Wheel> {
protected:
  /** @see WheelElement */
  static const WheelElement wheel_[8 * 8];
  Modulo30Wheel(const SieveOfEratosthenes* soe) :
    ModuloWheel<30, 8, init30Wheel> (soe) {
  }
  ~Modulo30Wheel() {
  }
};

/**
 * Implementation of a modulo 210 wheel (4th wheel).
 * EratMedium and EratBig are derived from Modulo210Wheel and use its
 * wheel_ array to skip multiples of 2, 3, 5 and 7.
 */
class Modulo210Wheel: protected ModuloWheel<210, 48, init210Wheel> {
protected:
  /** @see WheelElement */
  static const WheelElement wheel_[48 * 8];
  Modulo210Wheel(const SieveOfEratosthenes* soe) :
    ModuloWheel<210, 48, init210Wheel> (soe) {
  }
  ~Modulo210Wheel() {
  }
};

#endif /* WHEELFACTORIZATION_H */
