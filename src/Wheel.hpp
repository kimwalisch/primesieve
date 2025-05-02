///
/// @file   Wheel.hpp
/// @brief  Wheel factorization is used to skip multiles of
///         small primes in the sieve of Eratosthenes.
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef WHEEL_HPP
#define WHEEL_HPP

#include <primesieve/macros.hpp>

#include <stdint.h>
#include <algorithm>

namespace primesieve {

/// The WheelInit data structure is used to calculate the
/// first multiple >= start of each sieving prime.
///
struct WheelInit
{
  uint8_t nextMultipleFactor;
  uint8_t wheelIndex;
};

extern const WheelInit wheel30Init[30];
extern const WheelInit wheel210Init[210];

/// The abstract Wheel class is used skip multiples of small
/// primes in the sieve of Eratosthenes. The EratSmall,
/// EratMedium and EratBig classes are derived from Wheel.
///
template <int MODULO,
          int SIZE,
          int MAXMULTIPLEFACTOR,
          const WheelInit* INIT>
class Wheel
{
public:
  /// Add a new sieving prime to the sieving algorithm.
  /// Calculate the first multiple > segmentLow of prime and
  /// the position within the sieve array of that multiple
  /// and its wheel index. When done store the sieving prime.
  ///
  void addSievingPrime(uint64_t prime, uint64_t segmentLow)
  {
    ASSERT(segmentLow % 30 == 0);

    // This hack is required because in primesieve the 8
    // bits of each byte (of the sieve array) correspond to
    // the offsets { 7, 11, 13, 17, 19, 23, 29, 31 }.
    // So we are looking for: multiples > segmentLow + 6.
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

    multiple += nextMultiple;

    #if defined(ENABLE_ASSERT)
      if (MODULO >=    2) ASSERT(multiple %  2 != 0);
      if (MODULO >=    6) ASSERT(multiple %  3 != 0);
      if (MODULO >=   30) ASSERT(multiple %  5 != 0);
      if (MODULO >=  210) ASSERT(multiple %  7 != 0);
      if (MODULO >= 2310) ASSERT(multiple % 11 != 0);
    #endif

    uint64_t multipleIndex = (multiple - segmentLow) / 30;
    uint64_t wheelIndex = wheelOffsets_[prime % 30] + INIT[quotient % MODULO].wheelIndex;
    storeSievingPrime(prime, multipleIndex, wheelIndex);
  }

protected:
  uint64_t stop_ = 0;
  virtual ~Wheel() = default;
  virtual void storeSievingPrime(uint64_t, uint64_t, uint64_t) = 0;

  static uint64_t getMaxFactor()
  {
    return MAXMULTIPLEFACTOR;
  }

private:
  static const uint64_t wheelOffsets_[30];
};

template <int MODULO,
          int SIZE,
          int MAXMULTIPLEFACTOR,
          const WheelInit* INIT>
const uint64_t
Wheel<MODULO, SIZE, MAXMULTIPLEFACTOR, INIT>::wheelOffsets_[30] =
{
  0, SIZE * 7, 0, 0, 0, 0,
  0, SIZE * 0, 0, 0, 0, SIZE * 1,
  0, SIZE * 2, 0, 0, 0, SIZE * 3,
  0, SIZE * 4, 0, 0, 0, SIZE * 5,
  0, 0,        0, 0, 0, SIZE * 6
};

/// 3rd wheel, skips multiples of 2, 3 and 5
using Wheel30_t = Wheel<30, 8, 6, wheel30Init>;

/// 4th wheel, skips multiples of 2, 3, 5 and 7
using Wheel210_t = Wheel<210, 48, 10, wheel210Init>;

} // namespace

#endif
