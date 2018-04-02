///
/// @file   Wheel.hpp
/// @brief  Wheel factorization is used to skip multiles of
///         small primes in the sieve of Eratosthenes.
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef WHEEL_HPP
#define WHEEL_HPP

#include "config.hpp"
#include "primesieve_error.hpp"
#include "Bucket.hpp"

#include <stdint.h>
#include <algorithm>
#include <cassert>
#include <string>

namespace primesieve {

/// The WheelInit data structure is used to calculate the
/// first multiple >= start of each sieving prime
///
struct WheelInit
{
  uint8_t nextMultipleFactor;
  uint8_t wheelIndex;
};

extern const WheelInit wheel30Init[30];
extern const WheelInit wheel210Init[210];

/// The WheelElement data structure is used to skip multiples
/// of small primes using wheel factorization
///
struct WheelElement
{
  /// Bitmask used to unset the bit corresponding to the current
  /// multiple of a SievingPrime object
  uint8_t unsetBit;
  /// Factor used to calculate the next multiple of a sieving prime
  /// that is not divisible by any of the wheel factors
  uint8_t nextMultipleFactor;
  /// Overflow needed to correct the next multiple index
  /// (due to sievingPrime = prime / 30)
  uint8_t correct;
  /// Used to calculate the next wheel index:
  /// wheelIndex += next;
  int8_t next;
};

extern const WheelElement wheel30[8*8];
extern const WheelElement wheel210[48*8];

/// The abstract Wheel class is used skip multiples of small
/// primes in the sieve of Eratosthenes. The EratSmall,
/// EratMedium and EratBig classes are derived from Wheel.
///
template <int MODULO, int SIZE, const WheelElement* WHEEL, const WheelInit* INIT>
class Wheel
{
public:
  /// Add a new sieving prime to the sieving algorithm.
  /// Calculate the first multiple > segmentLow of prime and the
  /// position within the sieve array of that multiple
  /// and its wheel index. When done store the sieving prime.
  ///
  void addSievingPrime(uint64_t prime, uint64_t segmentLow)
  {
    assert(segmentLow % 30 == 0);
    segmentLow += 6;
    // calculate the first multiple (of prime) > segmentLow
    uint64_t quotient = (segmentLow / prime) + 1;
    quotient = std::max(prime, quotient);
    uint64_t multiple = prime * quotient;
    // prime not needed for sieving
    if (multiple > stop_ ||
        multiple < segmentLow)
      return;
    // calculate the next multiple of prime that is not
    // divisible by any of the wheel's factors
    uint64_t nextMultipleFactor = INIT[quotient % MODULO].nextMultipleFactor;
    uint64_t nextMultiple = prime * nextMultipleFactor;
    if (nextMultiple > stop_ - multiple)
      return;
    nextMultiple += multiple - segmentLow;
    uint64_t multipleIndex = nextMultiple / 30;
    uint64_t wheelIndex = wheelOffsets_[prime % 30] + INIT[quotient % MODULO].wheelIndex;
    storeSievingPrime(prime, multipleIndex, wheelIndex);
  }

protected:
  void init(uint64_t stop, uint64_t sieveSize)
  {
    stop_ = stop;
    uint64_t maxSieveSize = SievingPrime::MAX_MULTIPLEINDEX + 1;

    if (sieveSize > maxSieveSize)
      throw primesieve_error("Wheel: sieveSize must be <= " + std::to_string(maxSieveSize));
  }

  virtual ~Wheel()
  { }

  virtual void storeSievingPrime(uint64_t, uint64_t, uint64_t) = 0;

  static uint64_t getMaxFactor()
  {
    return WHEEL[0].nextMultipleFactor;
  }

  /// Cross-off the current multiple of sievingPrime
  /// and calculate its next multiple
  ///
  static void unsetBit(byte_t* sieve,
                       uint64_t sievingPrime,
                       uint64_t* multipleIndex,
                       uint64_t* wheelIndex)
  {
    sieve[*multipleIndex] &= WHEEL[*wheelIndex].unsetBit;
    *multipleIndex        += WHEEL[*wheelIndex].nextMultipleFactor * sievingPrime;
    *multipleIndex        += WHEEL[*wheelIndex].correct;
    *wheelIndex           += WHEEL[*wheelIndex].next;
  }

private:
  static const uint64_t wheelOffsets_[30];
  uint64_t stop_;
};

template <int MODULO, int SIZE, const WheelElement* WHEEL, const WheelInit* INIT>
const uint64_t
Wheel<MODULO, SIZE, WHEEL, INIT>::wheelOffsets_[30] =
{
  0, SIZE * 7, 0, 0, 0, 0,
  0, SIZE * 0, 0, 0, 0, SIZE * 1,
  0, SIZE * 2, 0, 0, 0, SIZE * 3,
  0, SIZE * 4, 0, 0, 0, SIZE * 5,
  0, 0,        0, 0, 0, SIZE * 6
};

/// 3rd wheel, skips multiples of 2, 3 and 5
typedef Wheel<30, 8, wheel30, wheel30Init> Wheel30_t;

/// 4th wheel, skips multiples of 2, 3, 5 and 7
typedef Wheel<210, 48, wheel210, wheel210Init> Wheel210_t;

} // namespace

#endif
