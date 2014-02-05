///
/// @file   WheelFactorization.hpp
/// @brief  Classes and structs related to wheel factorization.
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef WHEELFACTORIZATION_HPP
#define WHEELFACTORIZATION_HPP

#include "config.hpp"
#include "toString.hpp"
#include "pmath.hpp"
#include "primesieve_error.hpp"

#include <stdint.h>
#include <string>
#include <limits>
#include <cstddef>
#include <cassert>

namespace primesieve {

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
  /// Bitmask used to unset the bit corresponding to the current
  /// multiple of a SievingPrime object.
  uint8_t unsetBit;
  /// Factor used to calculate the next multiple of a sieving prime
  /// that is not divisible by any of the wheel factors.
  uint8_t nextMultipleFactor;
  /// Overflow needed to correct the next multiple index
  /// (due to sievingPrime = prime / 30).
  uint8_t correct;
  /// Used to calculate the next wheel index:
  /// wheelIndex += next;
  int8_t next;
};

extern const WheelElement wheel30[8*8];
extern const WheelElement wheel210[48*8];

/// Sieving primes are used to cross-off multiples (of itself).
/// Each SievingPrime object contains a sieving prime and the position
/// of its next multiple within the SieveOfEratosthenes array
/// (i.e. multipleIndex) and a wheelIndex.
///
class SievingPrime {
public:
  enum {
    MAX_MULTIPLEINDEX = (1 << 23) - 1,
    MAX_WHEELINDEX    = (1 <<  9) - 1
  };
  uint_t getSievingPrime() const
  {
    return sievingPrime_;
  }
  uint_t getMultipleIndex() const
  {
    return indexes_ & MAX_MULTIPLEINDEX;
  }
  uint_t getWheelIndex() const
  {
    return indexes_ >> 23;
  }
  void setMultipleIndex(uint_t multipleIndex)
  {
    assert(multipleIndex <= MAX_MULTIPLEINDEX);
    indexes_ = static_cast<uint32_t>(indexes_ | multipleIndex);
  }
  void setWheelIndex(uint_t wheelIndex)
  {
    assert(wheelIndex <= MAX_WHEELINDEX);
    indexes_ = static_cast<uint32_t>(wheelIndex << 23);
  }
  void set(uint_t multipleIndex,
           uint_t wheelIndex)
  {
    assert(multipleIndex <= MAX_MULTIPLEINDEX);
    assert(wheelIndex    <= MAX_WHEELINDEX);
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
  /// wheelIndex = 9 most significant bits of indexes_.
  uint32_t indexes_;
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
  SievingPrime* begin() { return &sievingPrimes_[0]; }
  SievingPrime* last()  { return &sievingPrimes_[config::BUCKETSIZE - 1]; }
  SievingPrime* end()   { return prime_; }
  Bucket* next()        { return next_; }
  bool hasNext() const  { return next_ != NULL; }
  bool empty()          { return begin() == end(); }
  void reset()          { prime_ = begin(); }
  void setNext(Bucket* next)
  {
    next_ = next;
  }
  /// Store a sieving prime in the bucket.
  /// @return false if the bucket is full else true.
  ///
  bool store(uint_t sievingPrime,
             uint_t multipleIndex,
             uint_t wheelIndex)
  {
    prime_->set(sievingPrime, multipleIndex, wheelIndex);
    return prime_++ != last();
  }
private:
  SievingPrime* prime_;
  Bucket* next_;
  SievingPrime sievingPrimes_[config::BUCKETSIZE];
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
  void addSievingPrime(uint_t prime, uint64_t segmentLow)
  {
    segmentLow += 6;
    // calculate the first multiple (of prime) > segmentLow
    uint64_t quotient = segmentLow / prime + 1;
    uint64_t multiple = prime * quotient;
    // prime not needed for sieving
    if (multiple > stop_)
      return;
    uint64_t square = isquare<uint64_t>(prime);
    if (multiple < square) {
      quotient = prime;
      multiple = square;
    }
    // calculate the next multiple of prime that is not
    // divisible by any of the wheel's factors
    uint64_t nextMultipleFactor = INIT[quotient % MODULO].nextMultipleFactor;
    multiple += prime * nextMultipleFactor;
    if (multiple > stop_)
      return;
    uint64_t lowOffset = multiple - segmentLow;
    uint_t multipleIndex = static_cast<uint_t>(lowOffset / NUMBERS_PER_BYTE);
    uint_t wheelIndex = wheelOffsets_[prime % NUMBERS_PER_BYTE] + INIT[quotient % MODULO].wheelIndex;
    storeSievingPrime(prime, multipleIndex, wheelIndex);
  }
protected:
  /// @param stop       Upper bound for sieving.
  /// @param sieveSize  Sieve size in bytes.
  ///
  WheelFactorization(uint64_t stop, uint_t sieveSize) :
    stop_(stop)
  {
    const uint_t maxSieveSize = SievingPrime::MAX_MULTIPLEINDEX + 1;
    if (sieveSize > maxSieveSize)
      throw primesieve_error("WheelFactorization: sieveSize must be <= " + toString(maxSieveSize));
    if (stop > getMaxStop())
      throw primesieve_error("WheelFactorization: stop must be <= " + getMaxStopString());
  }
  virtual ~WheelFactorization() { }
  virtual void storeSievingPrime(uint_t, uint_t, uint_t) = 0;
  static uint_t getMaxFactor()
  {
    return WHEEL[0].nextMultipleFactor;
  }
  /// Cross-off the current multiple (unset bit) of sievingPrime and
  /// calculate its next multiple i.e. multipleIndex.
  ///
  static void unsetBit(byte_t* sieve, uint_t sievingPrime, uint_t* multipleIndex, uint_t* wheelIndex)
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
  0, SIZE * 7, 0, 0, 0, 0,
  0, SIZE * 0, 0, 0, 0, SIZE * 1,
  0, SIZE * 2, 0, 0, 0, SIZE * 3,
  0, SIZE * 4, 0, 0, 0, SIZE * 5,
  0, 0,        0, 0, 0, SIZE * 6
};

/// 3rd wheel, skips multiples of 2, 3 and 5
typedef WheelFactorization<30, 8, wheel30Init, wheel30> Modulo30Wheel_t;

/// 4th wheel, skips multiples of 2, 3, 5 and 7
typedef WheelFactorization<210, 48, wheel210Init, wheel210> Modulo210Wheel_t;

} // namespace primesieve

#endif
