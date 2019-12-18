///
/// @file  Erat.hpp
///
/// Copyright (C) 2019 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef ERAT_HPP
#define ERAT_HPP

#include "forward.hpp"
#include "EratSmall.hpp"
#include "EratMedium.hpp"
#include "EratBig.hpp"
#include "noinline.hpp"

#include <stdint.h>
#include <array>
#include <memory>

namespace primesieve {

class PreSieve;

/// The abstract Erat class sieves primes using the segmented sieve
/// of Eratosthenes. It uses a bit array for sieving, the bit array
/// uses 8 flags for 30 numbers. Erat uses 3 different sieve of
/// Eratosthenes algorithms optimized for small, medium and big
/// sieving primes to cross-off multiples.
///
class Erat
{
public:
  uint64_t getSieveSize() const;
  uint64_t getStop() const;

protected:
  /// Sieve primes >= start_
  uint64_t start_ = 0;
  /// Sieve primes <= stop_
  uint64_t stop_ = 0;
  /// Size of sieve_ in bytes (power of 2)
  uint64_t sieveSize_ = 0;
  /// Lower bound of the current segment
  uint64_t segmentLow_ = ~0ull;
  /// Upper bound of the current segment
  uint64_t segmentHigh_ = 0;
  /// Sieve of Eratosthenes array
  uint8_t* sieve_ = nullptr;
  Erat();
  Erat(uint64_t, uint64_t);
  void init(uint64_t, uint64_t, uint64_t, PreSieve&);
  void addSievingPrime(uint64_t);
  NOINLINE void sieveSegment();
  bool hasNextSegment() const;
  static uint64_t nextPrime(uint64_t, uint64_t);

private:
  uint64_t maxPreSieve_ = 0;
  uint64_t maxEratSmall_ = 0;
  uint64_t maxEratMedium_ = 0;
  std::unique_ptr<uint8_t[]> deleter_;
  PreSieve* preSieve_ = nullptr;
  EratSmall eratSmall_;
  EratBig eratBig_;
  EratMedium eratMedium_;
  static uint64_t byteRemainder(uint64_t);
  void initSieve(uint64_t);
  void initErat();
  void preSieve();
  void crossOff();
  void sieveLastSegment();
};

/// Find the first set bit and calculate
/// the corresponding prime.
///
inline uint64_t Erat::nextPrime(uint64_t bits, uint64_t low)
{
  // Calculate bitValues[bitScanForward(bits)] using a custom
  // De Bruijn bitscan (that directly computes the bitValue
  // without ever computing the bitIndex). For primesieve's
  // use case this is as fast as the bsf or tzcnt instructions
  // on x64 but more portable.
  // https://www.chessprogramming.org/BitScan#De_Bruijn_Multiplication
  uint64_t debruijn = 0x3F08A4C6ACB9DBDull;
  uint64_t hash = ((bits ^ (bits - 1)) * debruijn) >> 58;
  uint64_t bitValue = bruijnBitValues[hash];
  uint64_t prime = low + bitValue;
  return prime;
}

inline void Erat::addSievingPrime(uint64_t prime)
{
       if (prime > maxEratMedium_)   eratBig_.addSievingPrime(prime, segmentLow_);
  else if (prime > maxEratSmall_) eratMedium_.addSievingPrime(prime, segmentLow_);
  else /* (prime > maxPreSieve) */ eratSmall_.addSievingPrime(prime, segmentLow_);
}

inline uint64_t Erat::getStop() const
{
  return stop_;
}

/// Sieve size in KiB
inline uint64_t Erat::getSieveSize() const
{
  return sieveSize_ >> 10;
}

} // namespace

#endif
