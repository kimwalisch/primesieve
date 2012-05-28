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


/// @file WheelFactorization.h
/// @brief Contains classes and structs related to wheel factorization.
/// Wheel factorization is used to skip multiples of small primes to
/// speed up the sieve of Eratosthenes.
/// http://en.wikipedia.org/wiki/Wheel_factorization

#ifndef WHEELFACTORIZATION_H
#define WHEELFACTORIZATION_H

#include "config.h"
#include "SieveOfEratosthenes.h"
#include "imath.h"

#include <stdint.h>
#include <cassert>
#include <stdexcept>
#include <sstream>

namespace soe {

/// WheelPrime objects are sieving primes <= sqrt(n) for use with wheel
/// factorization (skips multiples of small primes). EratSmall,
/// EratMedium and EratBig objects use WheelPrimes to cross-off
/// multiples. Each WheelPrime object contains the sieving prime, the
/// position of the next multiple within the SieveOfEratosthenes array
/// (multipleIndex) and a wheel index.
/// @remark WheelPrime  Uses 8 bytes per sieving prime.
///
class WheelPrime {
public:
  uint_t getSievingPrime() const
  {
    return sievingPrime_;
  }
  uint_t getMultipleIndex() const
  {
    return indexes_ & ((1u << 23) - 1);
  }
  uint_t getWheelIndex() const
  {
    return indexes_ >> 23;
  }
  void set(uint_t sievingPrime,
           uint_t multipleIndex,
           uint_t wheelIndex)
  {
    setIndexes(multipleIndex, wheelIndex);
    sievingPrime_ = static_cast<uint32_t>(sievingPrime);
  }
  void setIndexes(uint_t multipleIndex,
                  uint_t wheelIndex)
  {
    indexes_ = static_cast<uint32_t>(multipleIndex | (wheelIndex << 23));
  }
  void setWheelIndex(uint_t wheelIndex)
  {
    indexes_ = static_cast<uint32_t>(wheelIndex << 23);
  }
  void setMultipleIndex(uint_t multipleIndex)
  {
    indexes_ = static_cast<uint32_t>(indexes_ | multipleIndex);
  }
private:
  /// multipleIndex = 23 least significant bits of indexes_.
  /// wheelIndex    =  9 most  significant bits of indexes_.
  /// Packing multipleIndex and wheelIndex into the same 32-bit dword
  /// reduces primesieve's memory usage by 20%.
  uint32_t indexes_;
  /// sievingPrime_ = prime / 30;
  /// '/ 30' is used as SieveOfEratosthenes objects use a bit array
  /// with 30 numbers per byte for sieving.
  /// @see Wheel::getWheelPrimeData()
  uint32_t sievingPrime_;
};

/// The Bucket data structure is used to store sieving primes. It is
/// designed as a singly linked list, once there is no more space in
/// the current bucket a new bucket node is allocated ...
/// @see http://www.ieeta.pt/~tos/software/prime_sieve.html
///
class Bucket {
public:
  Bucket() : current_(wheelPrimes_) { }
  // list::push_back(Bucket()) adds an empty
  // bucket without unnecessary copying
  Bucket(const Bucket&) : current_(wheelPrimes_) { }
  void reset()
  {
    current_ = wheelPrimes_;
  }
  WheelPrime* begin()
  {
    return wheelPrimes_;
  }
  WheelPrime* end()
  {
    return current_;
  }
  Bucket* next()
  {
    return next_;
  }
  void setNext(Bucket* next)
  {
    next_ = next;
  }
  bool isEmpty() const
  {
    return current_ == wheelPrimes_;
  }
  bool hasNext() const
  {
    return next_ != NULL;
  }
  /// Add a WheelPrime to the Bucket.
  /// @return false  If the bucket is full else true.
  bool addWheelPrime(uint_t sievingPrime,
                     uint_t multipleIndex,
                     uint_t wheelIndex)
  {
    WheelPrime* wPrime = current_;
    current_++;
    wPrime->set(sievingPrime, multipleIndex, wheelIndex);
    return (wPrime != &wheelPrimes_[config::BUCKETSIZE - 1]);
  }
private:
  WheelPrime* current_;
  Bucket* next_;
  WheelPrime wheelPrimes_[config::BUCKETSIZE];
};

/// Precomputed arrays of WheelInit objects are used to calculate the
/// first multiple >= START of each sieving prime that is not
/// divisible by any of the wheel's factors and its wheel index.
/// @see WheelFactorization.cpp
/// @see Wheel::getWheelPrimeData(...)
///
struct WheelInit {
  uint8_t nextMultipleFactor;
  uint8_t wheelIndex;
};

/// Precomputed arrays of WheelElement objects are used to add wheel
/// factorization to the sieve of Eratosthenes.
/// The WheelElement data structure holds the information needed to
/// unset the bit within the SieveOfEratosthenes array corresponding to
/// the current multiple of each sieving prime, a factor used to
/// calculate the prime's next multiple and an offset used to calculate
/// its next wheel index.
/// @see WheelFactorization.cpp
/// @see EratMedium::sieve(), EratBig::sieve()
///
struct WheelElement {
  WheelElement(uint8_t _unsetBit,
               uint8_t _nextMultipleFactor,
               uint8_t _correct,
                int8_t _next) :
    unsetBit(_unsetBit),
    nextMultipleFactor(_nextMultipleFactor),
    correct(_correct),
    next(_next) { }
  /// Bitmask used with the '&' operator to unset the bit corresponding
  /// to the current multiple of a WheelPrime object.
  uint8_t unsetBit;
  /// Factor used to calculate the next multiple of a WheelPrime
  /// object that is not divisible by any of the wheel factors (e.g.
  /// not a multiple of 2, 3, and 5 for a modulo 30 wheel).
  uint8_t nextMultipleFactor;
  /// Overflow needed to correct (because of sievingPrime = prime / 30)
  /// the next multiple offset i.e.
  /// multipleIndex += sievingPrime * nextMultipleFactor + correct;
  uint8_t correct;
  /// Offset that is used to calculate the next wheel index of a
  /// WheelPrime object i.e. wheelIndex += next;
   int8_t next;
};

/// The abstract Wheel class provides functionality needed to use wheel
/// factorization with the sieve of Eratosthenes. The EratSmall,
/// EratMedium and EratBig classes are derived from Wheel.
/// Via template arguments it is possible to build different types of
/// Wheel classes e.g. Modulo30Wheel_t and Modulo210Wheel_t. 
///
template<uint_t              WHEEL_MODULO,
         uint_t              WHEEL_SIZE,
         const WheelElement* WHEEL_ARRAY,
         const WheelInit*    WHEEL_INIT>
class Wheel {
private:
  static const uint_t wheelOffsets_[30];
  /// Reference to the parent SieveOfEratosthenes object
  const SieveOfEratosthenes& soe_;
  Wheel(const Wheel&);
  Wheel& operator=(const Wheel&);
protected:
  Wheel(const SieveOfEratosthenes& soe) : soe_(soe)
  {
    uint64_t maxSievingPrime = UINT32_MAX;
    uint64_t maxInitFactor   = WHEEL_INIT[2].nextMultipleFactor + 1;
    uint64_t limit           = UINT64_MAX - maxSievingPrime * maxInitFactor;
    // prevent 64-bit overflows of multiple in getWheelPrimeData()
    if (soe_.getStop() > limit) {
      std::ostringstream error;
      error << "Wheel: stop must be <= (2^64-1) - (2^32-1) * "
            << maxInitFactor
            << ".";
      throw std::overflow_error(error.str());
    }
    // max(sieveSize) = max(WheelPrime::getMultipleIndex()) + 1 = 2^23
    if (soe_.getSieveSize() > (1U << 23))
      throw std::overflow_error("Wheel: sieveSize must be <= 2^23, 8192 kilobytes.");
  }
  ~Wheel() { }
  /// Used to initialize sieving primes <= sqrt(n) for use with the
  /// segmented sieve of Eratosthenes with wheel factorization.
  /// Calculates the first multiple >= segmentLow of prime that is not
  /// divisible by any of the wheel's prime factors (e.g. 2, 3 and 5
  /// for a modulo 30 wheel) and the position within the
  /// SieveOfEratosthenes array (multipleIndex) of that multiple and
  /// its wheel index.
  /// @return true  if the WheelPrime must be stored for sieving
  ///               (next multiple <= STOP) else false.
  ///
  bool getWheelPrimeData(uint64_t segmentLow,
                         uint_t* prime,
                         uint_t* multipleIndex,
                         uint_t* wheelIndex) const
  {
    // correction for primes of type i*30 + 31
    segmentLow += 6;
    // calculate the first multiple > segmentLow of prime
    uint64_t quotient = segmentLow / *prime + 1;
    uint64_t multiple = *prime * quotient;
    if (multiple > soe_.getStop())
      return false;
    const uint64_t square = isquare<uint64_t>(*prime);
    // prime^2 is the first multiple of prime
    // that needs to be crossed-off
    if (multiple < square) {
      multiple = square;
      quotient = *prime;
    }
    // calculate the next multiple that is not divisible by any of the
    // wheel's primes (e.g. 2, 3 and 5 for a modulo 30 wheel)
    multiple += static_cast<uint64_t>(*prime) * WHEEL_INIT[quotient % WHEEL_MODULO].nextMultipleFactor;
    if (multiple > soe_.getStop())
      return false;
    *multipleIndex = static_cast<uint_t>((multiple - segmentLow) / 30);
    *wheelIndex = wheelOffsets_[*prime % 30] + WHEEL_INIT[quotient % WHEEL_MODULO].wheelIndex;
    *prime /= 30;
    return true;
  }
  const WheelElement& wheel(uint_t index) const
  {
    assert(index < WHEEL_SIZE * 8);
    return WHEEL_ARRAY[index];
  }
};

/// The wheelOffsets_ array is used to calculate the index of the
/// first multiple >= START within the WHEEL_ARRAY. In primesieve there
/// are eight modulo 30 residue classes of sieving primes i.e.
/// i * 30 + k with k = { 1, 7, 11, 13, 17, 19, 23, 29 }, thus there
/// are also 8 wheel offsets. (In fact wheel30Array and wheel210Array
/// contain 8 wheels, one for each residue class)
///
template<uint_t              WHEEL_MODULO,
         uint_t              WHEEL_SIZE,
         const WheelElement* WHEEL_ARRAY,
         const WheelInit*    WHEEL_INIT>
const uint_t
Wheel<WHEEL_MODULO, WHEEL_SIZE, WHEEL_ARRAY, WHEEL_INIT>::wheelOffsets_[30] = {
        0xFF, 7 * WHEEL_SIZE, 0xFF, 0xFF, 0xFF,           0xFF,
        0xFF, 0 * WHEEL_SIZE, 0xFF, 0xFF, 0xFF, 1 * WHEEL_SIZE,
        0xFF, 2 * WHEEL_SIZE, 0xFF, 0xFF, 0xFF, 3 * WHEEL_SIZE,
        0xFF, 4 * WHEEL_SIZE, 0xFF, 0xFF, 0xFF, 5 * WHEEL_SIZE,
        0xFF,           0xFF, 0xFF, 0xFF, 0xFF, 6 * WHEEL_SIZE };

/// @see WheelFactorization.cpp
extern const WheelInit wheel30Init[30];
extern const WheelInit wheel210Init[210];
extern const WheelElement wheel30Array[8*8];
extern const WheelElement wheel210Array[48*8];

/// 3rd wheel, skips multiples of 2, 3 and 5
typedef Wheel<30, 8, wheel30Array, wheel30Init> Modulo30Wheel_t;
/// 4th wheel, skips multiples of 2, 3, 5 and 7
typedef Wheel<210, 48, wheel210Array, wheel210Init> Modulo210Wheel_t;

} // namespace soe

#endif
