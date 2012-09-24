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

/// @file   WheelFactorization.h
/// @brief  Classes and structs related to wheel factorization.
/// @see    http://en.wikipedia.org/wiki/Wheel_factorization
///
/// Wheel factorization is used to skip multiples of small primes to
/// speed up the sieve of Eratosthenes.

#ifndef WHEELFACTORIZATION_H
#define WHEELFACTORIZATION_H

#include "PrimeSieve.h"
#include "toString.h"
#include "imath.h"
#include "config.h"

#include <stdint.h>
#include <string>
#include <limits>
#include <cassert>

namespace soe {

/// The WheelInit data structure is used to calculate the first
/// multiple >= start of each sieving prime.
///
struct WheelInit {
  uint8_t nextMultipleFactor;
  uint8_t wheelIndex;
};

extern const WheelInit wheel30Init[30];
extern const WheelInit wheel210Init[210];

/// The WheelElement data structure is used to skip multiples of
/// small primes using wheel factorization.
///
struct WheelElement {
  /// Bitmask used with the bitwise & operator to unset the bit
  /// corresponding to the current multiple of a WheelPrime object.
  uint8_t unsetBit;
  /// Factor used to calculate the next multiple of a sieving prime
  /// that is not divisible by any of the wheel factors.
  uint8_t nextMultipleFactor;
  /// Overflow needed to correct the next multiple index,
  /// due to sievingPrime = prime / 30.
  uint8_t correct;
  /// Used to calculate the next wheel index:
  /// wheelIndex += next;
   int8_t next;
};

/// Modulo 30 wheel array, skips multiples of 2, 3 and 5
extern const WheelElement wheel30[8*8];
/// Modulo 210 wheel array, skips multiples of 2, 3, 5 and 7
extern const WheelElement wheel210[48*8];

/// WheelPrime objects are sieving primes for use with wheel
/// factorization (skips multiples of small primes).
/// Each WheelPrime contains a sieving prime, the position of the next
/// multiple within the SieveOfEratosthenes array (multipleIndex)
/// and a wheelIndex.
///
class WheelPrime {
public:
  enum {
    MAX_MULTIPLE_INDEX = (1 << 23) - 1
  };
  uint_t getSievingPrime() const  { return sievingPrime_; }
  uint_t getMultipleIndex() const { return indexes_ & ((1 << 23) - 1); }
  uint_t getWheelIndex() const    { return indexes_ >> 23; }

  void setMultipleIndex(uint_t multipleIndex)
  {
    assert(multipleIndex < (1u << 23));
    indexes_ = static_cast<uint32_t>(indexes_ | multipleIndex);
  }
  void setWheelIndex(uint_t wheelIndex)
  {
    assert(wheelIndex < (1u << 9));
    indexes_ = static_cast<uint32_t>(wheelIndex << 23);
  }
  void set(uint_t multipleIndex, uint_t wheelIndex)
  {
    assert(multipleIndex < (1u << 23));
    assert(wheelIndex    < (1u << 9));
    indexes_ = static_cast<uint32_t>(multipleIndex | (wheelIndex << 23));
  }
  void set(uint_t sievingPrime, uint_t multipleIndex, uint_t wheelIndex)
  {
    set(multipleIndex, wheelIndex);
    sievingPrime_ = static_cast<uint32_t>(sievingPrime);
  }
private:
  /// multipleIndex = 23 least significant bits of indexes_.
  /// wheelIndex    =  9 most  significant bits of indexes_.
  uint32_t indexes_;
  /// sievingPrime_ = prime / 30;
  /// '/ 30' is used as SieveOfEratosthenes objects use a bit array
  /// with 30 numbers per byte for sieving.
  uint32_t sievingPrime_;
};

/// The Bucket data structure is used to store sieving primes.
/// @see http://www.ieeta.pt/~tos/software/prime_sieve.html
/// The Bucket class is designed as a singly linked list, once there
/// is no more space in the current Bucket a new Bucket node is
/// allocated.
///
class Bucket {
public:
  Bucket(const Bucket&) { reset(); }
  Bucket()              { reset(); }
  WheelPrime* begin()   { return &wheelPrimes_[0]; }
  WheelPrime* last()    { return &wheelPrimes_[config::BUCKETSIZE - 1]; }
  WheelPrime* end()     { return current_;}
  Bucket* next()        { return next_; }
  bool hasNext() const  { return next_ != NULL; }
  bool empty()          { return begin() == end(); }
  void reset()          { current_ = begin(); }
  void setNext(Bucket* next)
  {
    next_ = next;
  }
  /// Store a WheelPrime in the bucket.
  /// @return false if the bucket is full else true.
  ///
  bool store(uint_t sievingPrime,
             uint_t multipleIndex,
             uint_t wheelIndex)
  {
    WheelPrime* wPrime = current_;
    current_++;
    wPrime->set(sievingPrime, multipleIndex, wheelIndex);
    return wPrime != last();
  }
private:
  WheelPrime* current_;
  Bucket* next_;
  WheelPrime wheelPrimes_[config::BUCKETSIZE];
};

/// The abstract WheelFactorization class is used skip multiples of
/// small primes in the sieve of Eratosthenes. The EratSmall,
/// EratMedium and EratBig classes are derived from
/// WheelFactorization.
///
template <uint_t MODULO, uint_t SIZE, const WheelInit* INIT, const WheelElement* WHEEL>
class WheelFactorization {
public:
  /// Get the maximum upper bound for sieving
  static uint64_t getMaxStop()
  {
    const uint64_t MAX_UINT32 = std::numeric_limits<uint32_t>::max();
    const uint64_t MAX_UINT64 = std::numeric_limits<uint64_t>::max();

    return MAX_UINT64 - MAX_UINT32 * getMaxFactor();
  }
  static std::string getMaxStopString()
  {
    return "2^64 - 2^32 * " + toString(getMaxFactor());
  }
  /// @brief Add a new sieving prime.
  ///
  /// Calculate the first multiple > segmentLow of prime and the
  /// position within the SieveOfEratosthenes array of that multiple
  /// and its wheel index. When done store the sieving prime.
  ///
  void add(uint_t prime, uint64_t segmentLow)
  {
    segmentLow += 6;
    // calculate the first multiple > segmentLow
    uint64_t quotient = segmentLow / prime + 1;
    uint64_t multiple = prime * quotient;
    // prime is not needed for sieving
    if (multiple > stop_)
      return;
    uint64_t square = isquare<uint64_t>(prime);
    if (multiple < square) {
      // prime^2 is the first multiple that must be crossed-off
      multiple = square;
      quotient = prime;
    }
    // calculate the next multiple of prime that is not
    // divisible by any of the wheel's factors
    multiple += static_cast<uint64_t>(prime) * INIT[quotient % MODULO].nextMultipleFactor;
    if (multiple > stop_)
      return;
    uint_t multipleIndex = static_cast<uint_t>((multiple - segmentLow) / 30);
    uint_t wheelIndex = wheelOffsets_[prime % 30] + INIT[quotient % MODULO].wheelIndex;
    // @see Erat(Small|Medium|Big).cpp
    store(prime, multipleIndex, wheelIndex);
  }
protected:
  /// @param stop       Upper bound for sieving.
  /// @param sieveSize  Sieve size in bytes.
  ///
  WheelFactorization(uint64_t stop, uint_t sieveSize) :
    stop_(stop)
  {
    const uint_t maxSieveSize = WheelPrime::MAX_MULTIPLE_INDEX + 1;
    if (sieveSize > maxSieveSize)
      throw primesieve_error("WheelFactorization: sieveSize must be <= " + toString(maxSieveSize));
    if (stop > getMaxStop())
      throw primesieve_error("WheelFactorization: stop must be <= " + getMaxStopString());
  }
  virtual ~WheelFactorization() { }
  virtual void store(uint_t, uint_t, uint_t) = 0;
  static uint_t getMaxFactor()
  {
    return WHEEL[0].nextMultipleFactor;
  }
  /// Cross-off the current multiple (unset bit) of sievingPrime and
  /// calculate its next multiple i.e. multipleIndex.
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
  const uint64_t stop_;
  DISALLOW_COPY_AND_ASSIGN(WheelFactorization);
};

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

/// 3rd wheel, skips multiples of 2, 3 and 5
typedef WheelFactorization<30, 8, wheel30Init, wheel30> Modulo30Wheel_t;
/// 4th wheel, skips multiples of 2, 3, 5 and 7
typedef WheelFactorization<210, 48, wheel210Init, wheel210> Modulo210Wheel_t;

} // namespace soe

#endif
