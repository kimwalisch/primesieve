///
/// @file  SieveOfEratosthenes.hpp
///
/// Copyright (C) 2014 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef SIEVEOFERATOSTHENES_HPP
#define SIEVEOFERATOSTHENES_HPP

#include "config.hpp"

#include <stdint.h>
#include <string>

namespace primesieve {

class PreSieve;
class EratSmall;
class EratMedium;
class EratBig;

/// @brief  The abstract SieveOfEratosthenes class sieves primes using
///         the segmented sieve of Eratosthenes.
///
/// SieveOfEratosthenes uses a bit array for sieving, the bit array
/// uses 8 flags for 30 numbers. SieveOfEratosthenes uses three
/// different sieve of Eratosthenes algorithms optimized for small,
/// medium and big sieving primes to cross-off multiples.
///
class SieveOfEratosthenes {
public:
  static uint64_t getMaxStop();
  static std::string getMaxStopString();
  uint64_t getStart() const;
  uint64_t getStop() const;
  uint_t getSqrtStop() const;
  uint_t getSieveSize() const;
  uint_t getPreSieve() const;
  void addSievingPrime(uint_t);
  void sieve();
protected:
  SieveOfEratosthenes(uint64_t, uint64_t, uint_t);
  virtual ~SieveOfEratosthenes();
  virtual void segmentFinished(const byte_t*, uint_t) = 0;
  static uint64_t getNextPrime(uint64_t*, uint64_t);
  uint64_t getSegmentLow() const;
private:
  static const uint_t bitValues_[8];
  static const uint_t bruijnBitValues_[64];
  /// Lower bound of the current segment
  uint64_t segmentLow_;
  /// Upper bound of the current segment
  uint64_t segmentHigh_;
  /// Sieve primes >= start_
  const uint64_t start_;
  /// Sieve primes <= stop_
  const uint64_t stop_;
  /// sqrt(stop_)
  uint_t sqrtStop_;
  /// Copy of preSieve_->getLimit()
  uint_t limitPreSieve_;
  /// Copy of eratSmall_->getLimit()
  uint_t limitEratSmall_;
  /// Copy of eratMedium_->getLimit()
  uint_t limitEratMedium_;
  /// Size of sieve_ in bytes (power of 2)
  uint_t sieveSize_;
  /// Sieve of Eratosthenes array
  byte_t* sieve_;
  /// Pre-sieve multiples of tiny sieving primes
  PreSieve* preSieve_;
  /// Cross-off multiples of small sieving primes
  EratSmall* eratSmall_;
  /// cross-off multiples of medium sieving primes
  EratMedium* eratMedium_;
  /// cross-off multiples of big sieving primes
  EratBig* eratBig_;
  static uint64_t getByteRemainder(uint64_t);
  void init();
  void cleanUp();
  void preSieve();
  void crossOffMultiples();
  void sieveSegment();
  DISALLOW_COPY_AND_ASSIGN(SieveOfEratosthenes);
};

} // namespace primesieve

#endif
