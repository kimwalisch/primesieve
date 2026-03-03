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

#include "Bucket.hpp"
#include "Wheel.hpp"

#include <primesieve/macros.hpp>
#include <primesieve/Vector.hpp>

#include <cstddef>
#include <stdint.h>

namespace primesieve {

/// EratMedium is an implementation of the segmented sieve
/// of Eratosthenes optimized for small sieving primes that
/// have many multiples per segment.
///
class EratMedium : public Wheel30_t
{
public:
  void init(uint64_t, uint64_t);
  void crossOff(Vector<uint8_t>& sieve);
  bool hasSievingPrimes() const;
private:
  uint64_t maxPrime_ = 0;
  std::size_t l1CacheSize_ = 0;
  Vector<SievingPrime> primeVectors_[8];
  void storeSievingPrime(uint64_t, uint64_t, uint64_t);
  NOINLINE void crossOff0(Vector<SievingPrime>&, Vector<uint8_t>&);
  NOINLINE void crossOff1(Vector<SievingPrime>&, Vector<uint8_t>&);
  NOINLINE void crossOff2(Vector<SievingPrime>&, Vector<uint8_t>&);
  NOINLINE void crossOff3(Vector<SievingPrime>&, Vector<uint8_t>&);
  NOINLINE void crossOff4(Vector<SievingPrime>&, Vector<uint8_t>&);
  NOINLINE void crossOff5(Vector<SievingPrime>&, Vector<uint8_t>&);
  NOINLINE void crossOff6(Vector<SievingPrime>&, Vector<uint8_t>&);
  NOINLINE void crossOff7(Vector<SievingPrime>&, Vector<uint8_t>&);
};

} // namespace

#endif
