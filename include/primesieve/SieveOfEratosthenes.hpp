///
/// @file  SieveOfEratosthenes.hpp
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef SIEVEOFERATOSTHENES_HPP
#define SIEVEOFERATOSTHENES_HPP

#include "config.hpp"
#include "EratSmall.hpp"
#include "EratMedium.hpp"
#include "EratBig.hpp"
#include "pmath.hpp"

#include <stdint.h>
#include <memory>

namespace primesieve {

class PreSieve;

/// The abstract SieveOfEratosthenes class sieves primes using
/// the segmented sieve of Eratosthenes. It uses a bit array
/// for sieving, the bit array uses 8 flags for 30 numbers.
/// SieveOfEratosthenes uses 3 different sieve of Eratosthenes
/// algorithms optimized for small, medium and big sieving primes
/// to cross-off multiples.
///
class SieveOfEratosthenes
{
public:
  uint64_t getStart() const;
  uint64_t getStop() const;
  uint_t getSqrtStop() const;
  uint_t getSieveSize() const;
  void addSievingPrime(uint_t);
  void sieve();
protected:
  SieveOfEratosthenes(uint64_t, uint64_t, uint_t, const PreSieve&);
  virtual ~SieveOfEratosthenes() { };
  virtual void generatePrimes(const byte_t*, uint_t) = 0;
  static uint64_t nextPrime(uint64_t*, uint64_t);
  uint64_t getSegmentLow() const;
private:
  static const uint_t bitValues_[8];
  static const uint_t bruijnBitValues_[64];
  /// Lower bound of the current segment
  uint64_t segmentLow_;
  /// Upper bound of the current segment
  uint64_t segmentHigh_;
  /// Sieve primes >= start_
  uint64_t start_;
  /// Sieve primes <= stop_
  uint64_t stop_;
  const PreSieve& preSieve_;
  /// sqrt(stop_)
  uint_t sqrtStop_;
  uint_t limitPreSieve_;
  uint_t limitEratSmall_;
  uint_t limitEratMedium_;
  /// Size of sieve_ in bytes (power of 2)
  uint_t sieveSize_;
  /// Sieve of Eratosthenes array
  byte_t* sieve_;
  std::unique_ptr<byte_t[]> deleteSieve_;
  std::unique_ptr<EratSmall> eratSmall_;
  std::unique_ptr<EratMedium> eratMedium_;
  std::unique_ptr<EratBig> eratBig_;
  static uint64_t getByteRemainder(uint64_t);
  void allocate();
  void preSieve();
  void crossOffMultiples();
  void sieveSegment();
};

/// Reconstruct the prime number corresponding to
/// the first set bit and unset that bit
///
inline uint64_t SieveOfEratosthenes::nextPrime(uint64_t* bits, uint64_t low)
{
  // calculate bitValues_[bitScanForward(*bits)]
  // using a custom De Bruijn bitscan
  uint64_t debruijn64 = 0x3F08A4C6ACB9DBDull;
  uint64_t mask = *bits - 1;
  uint64_t bitValue = bruijnBitValues_[((*bits ^ mask) * debruijn64) >> 58];
  uint64_t prime = low + bitValue;
  *bits &= mask;
  return prime;
}

/// This method is called consecutively for all
/// sieving primes up to sqrt(stop)
///
inline void SieveOfEratosthenes::addSievingPrime(uint_t prime)
{
  uint64_t square = isquare<uint64_t>(prime);

  // This loop is executed once all primes <= sqrt(segmentHigh)
  // required to sieve the next segment have been
  // added to the erat* objects further down
  while (segmentHigh_ < square)
    sieveSegment();

       if (prime > limitEratMedium_)   eratBig_->addSievingPrime(prime, segmentLow_);
  else if (prime > limitEratSmall_) eratMedium_->addSievingPrime(prime, segmentLow_);
  else /* (prime > limitPreSieve) */ eratSmall_->addSievingPrime(prime, segmentLow_);
}

inline uint64_t SieveOfEratosthenes::getStart() const
{
  return start_;
}

inline uint64_t SieveOfEratosthenes::getStop() const
{
  return stop_;
}

inline uint64_t SieveOfEratosthenes::getSegmentLow() const
{
  return segmentLow_;
}

inline uint_t SieveOfEratosthenes::getSieveSize() const
{
  return sieveSize_;
}

} // namespace

#endif
