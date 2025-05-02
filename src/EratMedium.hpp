///
/// @file  EratMedium.hpp
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef ERATMEDIUM_HPP
#define ERATMEDIUM_HPP

#include "Wheel.hpp"

#include <primesieve/macros.hpp>
#include <primesieve/Vector.hpp>

#include <stdint.h>

namespace primesieve {

class MemoryPool;
class SievingPrime;
class Bucket;

/// EratMedium is an implementation of the segmented sieve of
/// Eratosthenes optimized for medium sieving primes
/// that have a few multiples per segment.
///
class EratMedium : public Wheel30_t
{
public:
  void init(uint64_t, uint64_t, MemoryPool&);
  bool hasSievingPrimes() const { return !buckets_.empty(); }
  NOINLINE void crossOff(Vector<uint8_t>& sieve);
private:
  uint64_t maxPrime_ = 0;
  MemoryPool* memoryPool_ = nullptr;
  Vector<SievingPrime*> buckets_;
  Vector<SievingPrime*> currentBuckets_;
  void storeSievingPrime(uint64_t, uint64_t, uint64_t);
  NOINLINE void crossOff_7(uint8_t*, std::size_t, Bucket*);
  NOINLINE void crossOff_11(uint8_t*, std::size_t, Bucket*);
  NOINLINE void crossOff_13(uint8_t*, std::size_t, Bucket*);
  NOINLINE void crossOff_17(uint8_t*, std::size_t, Bucket*);
  NOINLINE void crossOff_19(uint8_t*, std::size_t, Bucket*);
  NOINLINE void crossOff_23(uint8_t*, std::size_t, Bucket*);
  NOINLINE void crossOff_29(uint8_t*, std::size_t, Bucket*);
  NOINLINE void crossOff_31(uint8_t*, std::size_t, Bucket*);
};

} // namespace

#endif
