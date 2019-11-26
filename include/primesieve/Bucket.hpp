///
/// @file   Bucket.hpp
/// @brief  A bucket is a container for sieving primes.
///         The Bucket class is designed as a singly linked list,
///         once there is no more space in the current Bucket
///         a new Bucket is allocated.
///
/// Copyright (C) 2019 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef BUCKET_HPP
#define BUCKET_HPP

#include "config.hpp"
#include "pmath.hpp"

#include <stdint.h>
#include <cassert>

namespace primesieve {

/// Each SievingPrime object contains a sieving prime and the
/// position of its next multiple inside the sieve array i.e.
/// multipleIndex and a wheelIndex. In order to reduce the memory
/// usage the multipleIndex and wheelIndex are packed into a
/// single 32-bit variable.
///
class SievingPrime
{
public:
  enum {
    MAX_MULTIPLEINDEX = (1 << 23) - 1,
    MAX_WHEELINDEX    = (1 << (32 - 23)) - 1
  };

  SievingPrime() = default;

  SievingPrime(uint64_t sievingPrime,
               uint64_t multipleIndex,
               uint64_t wheelIndex)
  {
    set(multipleIndex, wheelIndex);
    sievingPrime_ = (uint32_t) sievingPrime;
  }

  void set(uint64_t multipleIndex,
           uint64_t wheelIndex)
  {
    assert(multipleIndex <= MAX_MULTIPLEINDEX);
    assert(wheelIndex <= MAX_WHEELINDEX);

    indexes_ = (uint32_t) (multipleIndex | (wheelIndex << 23));
  }

  void set(uint64_t sievingPrime,
           uint64_t multipleIndex,
           uint64_t wheelIndex)
  {
    set(multipleIndex, wheelIndex);
    sievingPrime_ = (uint32_t) sievingPrime;
  }

  uint64_t getSievingPrime() const
  {
    return sievingPrime_;
  }

  uint64_t getMultipleIndex() const
  {
    return indexes_ & MAX_MULTIPLEINDEX;
  }

  uint64_t getWheelIndex() const
  {
    return indexes_ >> 23;
  }

  void setMultipleIndex(uint64_t multipleIndex)
  {
    assert(multipleIndex <= MAX_MULTIPLEINDEX);
    indexes_ = (uint32_t) (indexes_ | multipleIndex);
  }

  void setWheelIndex(uint64_t wheelIndex)
  {
    assert(wheelIndex <= MAX_WHEELINDEX);
    indexes_ = (uint32_t) (wheelIndex << 23);
  }

private:
  /// multipleIndex = 23 least significant bits of indexes_
  /// wheelIndex = 9 most significant bits of indexes_
  uint32_t indexes_;
  uint32_t sievingPrime_;
};

/// The Bucket data structure is used to store sieving primes.
/// @see http://www.ieeta.pt/~tos/software/prime_sieve.html
/// The Bucket class is designed as a singly linked list, once
/// there is no more space in the current Bucket a new Bucket
/// is allocated.
///
class Bucket
{
public:
  SievingPrime* begin() { return &sievingPrimes_[0]; }
  SievingPrime* end()   { return end_; }
  Bucket* next()        { return next_; }
  void setNext(Bucket* next) { next_ = next; }
  void setEnd(SievingPrime* end) { end_ = end; }
  void reset() { end_ = begin(); }

private:
  enum {
    SIEVING_PRIMES_OFFSET = sizeof(SievingPrime*) + sizeof(Bucket*),
    SIEVING_PRIMES_SIZE = (config::BUCKET_BYTES - SIEVING_PRIMES_OFFSET) / sizeof(SievingPrime)
  };

  SievingPrime* end_;
  Bucket* next_;
  SievingPrime sievingPrimes_[SIEVING_PRIMES_SIZE];
};

static_assert(isPow2(sizeof(Bucket)), "sizeof(Bucket) must be a power of 2!");

} // namespace

#endif
