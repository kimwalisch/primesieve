///
/// @file   Erat.hpp
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef ERAT_HPP
#define ERAT_HPP

#include "config.hpp"
#include "EratSmall.hpp"
#include "EratMedium.hpp"
#include "EratBig.hpp"

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
  byte_t* sieve_ = nullptr;
  PreSieve* preSieve_;
  Erat();
  Erat(uint64_t, uint64_t);
  virtual ~Erat();
  void init(uint64_t, uint64_t, uint64_t, PreSieve&);
  void addSievingPrime(uint64_t);
  void sieve(uint64_t);
  void sieve();
  void sieveSegment();
  bool hasNextSegment() const;
  virtual void print() { }
  static uint64_t nextPrime(uint64_t*, uint64_t);
private:
  static const std::array<uint64_t, 64> bruijnBitValues_;
  uint64_t maxPreSieve_;
  uint64_t maxEratSmall_;
  uint64_t maxEratMedium_;
  std::unique_ptr<byte_t[]> deleter_;
  EratSmall eratSmall_;
  EratMedium eratMedium_;
  EratBig eratBig_;
  static uint64_t byteRemainder(uint64_t);
  void initSieve(uint64_t);
  void initErat();
  void preSieve();
  void crossOff();
  void sieveLastSegment();
};

/// Reconstruct the prime number corresponding to
/// the first set bit and unset that bit
///
inline uint64_t Erat::nextPrime(uint64_t* bits, uint64_t low)
{
  // calculate bitValues_[bitScanForward(*bits)]
  // using a custom De Bruijn bitscan
  uint64_t debruijn = 0x3F08A4C6ACB9DBDull;
  uint64_t mask = *bits - 1;
  uint64_t bitValue = bruijnBitValues_[((*bits ^ mask) * debruijn) >> 58];
  uint64_t prime = low + bitValue;
  *bits &= mask;
  return prime;
}

/// This method is called consecutively for
/// all sieving primes up to sqrt(stop)
///
inline void Erat::sieve(uint64_t prime)
{
  // This loop is executed once all primes <= sqrt(segmentHigh)
  // required to sieve the next segment have been
  // added to the erat* objects further down
  while (prime * prime > segmentHigh_)
    sieveSegment();

  addSievingPrime(prime);
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

/// Sieve size in kilobytes
inline uint64_t Erat::getSieveSize() const
{
  return sieveSize_ / 1024;
}

} // namespace

#endif
