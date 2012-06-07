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
/// @brief Classes and structs related to wheel factorization.
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
    return indexes_ & ((1 << 23) - 1);
  }
  uint_t getWheelIndex() const
  {
    return indexes_ >> 23;
  }
  void setMultipleIndex(uint_t multipleIndex)
  {
    indexes_ = static_cast<uint32_t>(indexes_ | multipleIndex);
  }
  void setWheelIndex(uint_t wheelIndex)
  {
    indexes_ = static_cast<uint32_t>(wheelIndex << 23);
  }
  void set(uint_t multipleIndex,
           uint_t wheelIndex)
  {
    indexes_ = static_cast<uint32_t>(multipleIndex | (wheelIndex << 23));
  }
  void set(uint_t sievingPrime,
           uint_t multipleIndex,
           uint_t wheelIndex)
  {
    set(multipleIndex, wheelIndex);
    sievingPrime_ = static_cast<uint32_t>(sievingPrime);
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
  /// list::push_back(Bucket()) adds an empty bucket (no copying)
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
  /// Store a WheelPrime in the bucket.
  /// @return false  if the bucket is full else true.
  bool store(uint_t sievingPrime,
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
///
struct WheelElement {
  WheelElement(uint8_t _unsetBit,
               uint8_t _nextMultipleFactor,
               uint8_t _correct,
                int8_t _next) :
    unsetBit(_unsetBit),
    nextMultipleFactor(_nextMultipleFactor),
    correct(_correct),
    next(_next)
  { }
  /// Bitmask used with the '&' operator to unset the bit corresponding
  /// to the current multiple of a WheelPrime object.
  uint8_t unsetBit;
  /// Factor used to calculate the next multiple of a WheelPrime
  /// object that is not divisible by any of the wheel factors (e.g.
  /// not a multiple of 2, 3, and 5 for a modulo 30 wheel).
  uint8_t nextMultipleFactor;
  /// Overflow needed to correct (due to sievingPrime = prime / 30)
  /// the next multiple offset i.e.
  /// multipleIndex += sievingPrime * nextMultipleFactor + correct;
  uint8_t correct;
  /// Offset that is used to calculate the next wheel index of a
  /// WheelPrime object i.e. wheelIndex += next;
   int8_t next;
};

/// The abstract WheelFactorization class provides wheel factorization
/// functionality needed to skip multiples of small primes e.g. <= 7
/// in the sieve of Eratosthenes. The EratSmall, EratMedium and
/// EratBig classes are derived from WheelFactorization.
///
template <uint_t MODULO, uint_t SIZE, const WheelInit* INIT, const WheelElement* WHEEL>
class WheelFactorization {
public:
  /// Calculate the first multiple >= segmentLow of prime that is not
  /// divisible by any of the wheel's factors (e.g. 2, 3 and 5 for a
  /// modulo 30 wheel) and the position within the SieveOfEratosthenes
  /// array of that multiple (multipleIndex) and its wheel index.
  /// When done store the sieving prime.
  /// @see sieve() in SieveOfEratosthenes-inline.h
  ///
  void addSievingPrime(uint_t prime, uint64_t segmentLow)
  {
    segmentLow += 6;
    // calculate the first multiple > segmentLow
    uint64_t quotient = segmentLow / prime + 1;
    uint64_t multiple = prime * quotient;
    // prime is not needed for sieving
    if (multiple > soe_.getStop())
      return;
    uint64_t square = isquare<uint64_t>(prime);
    // first multiple that must be crossed-off is prime^2
    if (multiple < square) {
      multiple = square;
      quotient = prime;
    }
    // calculate the next multiple of prime that is not
    // divisible by any of the wheel's factors
    multiple += static_cast<uint64_t>(prime) * INIT[quotient % MODULO].nextMultipleFactor;
    if (multiple > soe_.getStop())
      return;
    uint_t multipleIndex = static_cast<uint_t>((multiple - segmentLow) / 30);
    uint_t wheelIndex = wheelOffsets_[prime % 30] + INIT[quotient % MODULO].wheelIndex;
    prime /= 30;
    // @see Erat(Small|Medium|Big).cpp
    storeSievingPrime(prime, multipleIndex, wheelIndex);
  }
protected:
  WheelFactorization(const SieveOfEratosthenes& soe) : soe_(soe)
  {
    uint64_t maxSievingPrime = UINT32_MAX;
    uint64_t limit           = UINT64_MAX - maxSievingPrime * getMaxFactor();
    // prevent 64-bit overflows of multiple in addSievingPrime()
    if (soe_.getStop() > limit) {
      std::ostringstream error;
      error << "WheelFactorization: stop must be <= (2^64-1) - (2^32-1) * "
            << getMaxFactor()
            << ".";
      throw std::overflow_error(error.str());
    }
    // max(sieveSize) = max(WheelPrime::getMultipleIndex()) + 1 = 2^23
    if (soe_.getSieveSize() > (1u << 23))
      throw std::overflow_error("WheelFactorization: sieveSize must be <= 2^23, 8192 kilobytes.");
  }
  virtual ~WheelFactorization() { }
  virtual void storeSievingPrime(uint_t, uint_t, uint_t) = 0;
  static uint_t getMaxFactor()
  {
    return WHEEL[0].nextMultipleFactor;
  }
  /// Cross-off the current multiple (unset bit) of sievingPrime and
  /// calculate its next multiple.
  ///
  static void unsetBit(uint8_t* sieve, uint_t sievingPrime, uint_t* multipleIndex, uint_t* wheelIndex)
  {
    sieve[*multipleIndex] &= WHEEL[*wheelIndex].unsetBit;
    *multipleIndex        += WHEEL[*wheelIndex].nextMultipleFactor * sievingPrime;
    *multipleIndex        += WHEEL[*wheelIndex].correct;
    *wheelIndex           += WHEEL[*wheelIndex].next;
  }
private:
  static const uint_t wheelOffsets_[30];
  /// Reference to the parent SieveOfEratosthenes object
  const SieveOfEratosthenes& soe_;
  WheelFactorization(const WheelFactorization&);
  WheelFactorization& operator=(const WheelFactorization&);
};

/// The wheelOffsets_ array is used to calculate the index of the
/// first multiple >= segmentLow within the WHEEL array. In primesieve
/// there are eight modulo 30 residue classes of sieving primes i.e.
/// k = { 1, 7, 11, 13, 17, 19, 23, 29 }, thus 8 wheel offsets.
///
template <uint_t MODULO, uint_t SIZE, const WheelInit* INIT, const WheelElement* WHEEL>
const uint_t
WheelFactorization<MODULO, SIZE, INIT, WHEEL>::wheelOffsets_[30] =
{
  0xFF, 7 * SIZE, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0 * SIZE, 0xFF, 0xFF, 0xFF, 1 * SIZE,
  0xFF, 2 * SIZE, 0xFF, 0xFF, 0xFF, 3 * SIZE,
  0xFF, 4 * SIZE, 0xFF, 0xFF, 0xFF, 5 * SIZE,
  0xFF, 0xFF,     0xFF, 0xFF, 0xFF, 6 * SIZE
};

/// @see WheelFactorization.cpp
extern const WheelInit    wheel30Init[30];
extern const WheelInit    wheel210Init[210];
extern const WheelElement wheel30[8*8];
extern const WheelElement wheel210[48*8];

/// 3rd wheel, skips multiples of 2, 3 and 5
typedef WheelFactorization<30, 8, wheel30Init, wheel30> Modulo30Wheel_t;
/// 4th wheel, skips multiples of 2, 3, 5 and 7
typedef WheelFactorization<210, 48, wheel210Init, wheel210> Modulo210Wheel_t;

} // namespace soe

#endif
