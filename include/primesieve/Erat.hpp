///
/// @file  Erat.hpp
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
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
#include "macros.hpp"
#include "intrinsics.hpp"
#include "pod_vector.hpp"

#include <stdint.h>

namespace primesieve {

class PreSieve;
class MemoryPool;

/// The abstract Erat class sieves primes using the segmented sieve
/// of Eratosthenes. It uses a bit array for sieving, the bit array
/// uses 8 flags for 30 numbers. Erat uses 3 different sieve of
/// Eratosthenes algorithms optimized for small, medium and big
/// sieving primes to cross-off multiples.
///
class Erat
{
public:
  uint64_t getStop() const;

protected:
  /// Sieve primes >= start_
  uint64_t start_ = 0;
  /// Sieve primes <= stop_
  uint64_t stop_ = 0;
  /// Lower bound of the current segment
  uint64_t segmentLow_ = ~0ull;
  /// Upper bound of the current segment
  uint64_t segmentHigh_ = 0;
  /// Sieve of Eratosthenes array
  pod_vector<uint8_t> sieve_;
  Erat() = default;
  Erat(uint64_t, uint64_t);
  void init(uint64_t, uint64_t, uint64_t, PreSieve&, MemoryPool& memoryPool);
  void addSievingPrime(uint64_t);
  NOINLINE void sieveSegment();
  bool hasNextSegment() const;
  static uint64_t nextPrime(uint64_t, uint64_t);

private:
  uint64_t maxPreSieve_ = 0;
  uint64_t maxEratSmall_ = 0;
  uint64_t maxEratMedium_ = 0;
  PreSieve* preSieve_ = nullptr;
  EratSmall eratSmall_;
  EratBig eratBig_;
  EratMedium eratMedium_;
  static uint64_t byteRemainder(uint64_t);
  static uint64_t getL1CacheSize();
  void initAlgorithms(uint64_t maxSieveSize, MemoryPool&);
  void preSieve();
  void crossOff();
  void sieveLastSegment();
};

/// Convert the 1st set bit into a prime number.
/// In order to reduce branch mispredictions nextPrime() may
/// be called with bits = 0 in which case nextPrime()
/// returns a random 64-bit integer. It is up to the caller
/// to handle this use case correctly.
///
inline uint64_t Erat::nextPrime(uint64_t bits, uint64_t low)
{
// CTZ64_SUPPORTS_ZERO is defined if (ctz64(0) <= 64),
// in this case we use the optimal code path.
#if defined(CTZ64_SUPPORTS_ZERO)
  auto bitIndex = ctz64(bits);
  assert(bitIndex < bitValues.size());
  uint64_t bitValue = bitValues[bitIndex];
#elif defined(HAS_CTZ64)
  // ctz64(0) is undefined behavior. To avoid undefined
  // behavior we set the highest bit.
  auto bitIndex = ctz64(bits | (1ull << 63));
  uint64_t bitValue = bitValues[bitIndex];
#else
  // Fallback if CTZ instruction is not avilable
  uint64_t debruijn = 0x3F08A4C6ACB9DBDull;
  uint64_t hash = ((bits ^ (bits - 1)) * debruijn) >> 58;
  uint64_t bitValue = bruijnBitValues[hash];
#endif

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

} // namespace

#endif
