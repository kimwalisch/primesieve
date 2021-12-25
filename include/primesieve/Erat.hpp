///
/// @file  Erat.hpp
///
/// Copyright (C) 2020 Kim Walisch, <kim.walisch@gmail.com>
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

#include <stdint.h>
#include <array>
#include <memory>

/// In order convert 1 bits of the sieve array into primes we
/// need to quickly calculate the index of the first set bit.
/// This CPU instruction is usually named CTZ (Count trailing
/// zeros). Unfortunately only x86 and x64 CPUs currently have
/// an instruction for that, other CPU architectures like
/// ARM64 or PPC64 have to emulate the CTZ instruction using
/// multiple other instructions.
///
/// On x64 CPUs there are actually 2 instructions to count the
/// number of trailing zeros: BSF and TZCNT. BSF is an old
/// instruction whereas TZCNT is much more recent (Bit
/// Manipulation Instruction Set 1). Since I expect BSF to be
/// slow on future x64 CPUs (because it is legacy) we only use
/// __builtin_ctzll() if we can guarantee that TZCNT will be
/// generated.
///
/// There is also a quick, pure integer algorithm known for
/// quickly computing the index of the 1st set bit. This
/// algorithm is named the "De Bruijn bitscan".
/// https://www.chessprogramming.org/BitScan
///
/// Because of this situation, we only use __builtin_ctzll()
/// or std::countr_zero() when we know that the user's CPU
/// architecture can quickly compute CTZ, either using a single
/// instruction or emulated using very few instructions. For
/// all other CPU architectures we fallback to the "De Bruijn
/// bitscan" algorithm.

#if !defined(__has_builtin)
  #define __has_builtin(x) 0
#endif

#if !defined(__has_include)
  #define __has_include(x) 0
#endif

#if __cplusplus >= 202002L && \
    __has_include(<bit>) && \
    (defined(__BMI__) /* TZCNT (x64) */ || \
     defined(__aarch64__) /* CTZ = RBIT + CLZ */ || \
     defined(_M_ARM64) /* CTZ = RBIT + CLZ */)

#include <bit>
#define ctz64(x) std::countr_zero(x)

#elif __has_builtin(__builtin_ctzll) && \
    (defined(__BMI__) /* TZCNT (x64) */ || \
     defined(__aarch64__) /* CTZ = RBIT + CLZ */ || \
     defined(_M_ARM64) /* CTZ = RBIT + CLZ */)

#define ctz64(x) __builtin_ctzll(x)

#endif

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
  uint64_t getL1CacheSize() const;
  void initSieve(uint64_t);
  void initErat();
  void preSieve();
  void crossOff();
  void sieveLastSegment();
};

/// Convert 1st set bit into prime
inline uint64_t Erat::nextPrime(uint64_t bits, uint64_t low)
{
#if defined(ctz64)
  // Find first set 1 bit

  // Some implementations of ctz64 include an explicit check for the
  // argument being 0. This is because, for example, the GCC version
  // of countr_zero is implemented in terms of __builtin_ctz, which is
  // undefined for 0. But since this function is never (*) called with
  // bits == 0, we can avoid that check and the associated (untaken)
  // conditional jump.
  //
  // (*) the proposed loop unrolling optimization in
  // PrimeGenerator::fill actually passes bits == 0 sometimes. On
  // processors where __builtin_ctzll is truly undefined for bits ==
  // 0, this may segfault.
  //  if (bits == 0) UNREACHABLE;
  auto bitIndex = ctz64(bits);
  uint64_t bitValue = bitValues[bitIndex];
  uint64_t prime = low + bitValue;
  return prime;
#else
  // Fallback if CTZ instruction is not avilable
  uint64_t debruijn = 0x3F08A4C6ACB9DBDull;
  uint64_t hash = ((bits ^ (bits - 1)) * debruijn) >> 58;
  uint64_t bitValue = bruijnBitValues[hash];
  uint64_t prime = low + bitValue;
  return prime;
#endif
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
