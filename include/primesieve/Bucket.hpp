///
/// @file   Bucket.hpp
/// @brief  A bucket is a container for sieving primes.
///         The Bucket class is designed as a singly linked list,
///         once there is no more space in the current Bucket
///         a new Bucket is allocated.
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef BUCKET_HPP
#define BUCKET_HPP

#include "config.hpp"
#include "macros.hpp"
#include "pmath.hpp"

#include <stdint.h>
#include <cstddef>

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

  SievingPrime(std::size_t sievingPrime,
               std::size_t multipleIndex,
               std::size_t wheelIndex)
  {
    set(sievingPrime, multipleIndex, wheelIndex);
  }

  void set(std::size_t multipleIndex,
           std::size_t wheelIndex)
  {
    ASSERT(multipleIndex <= MAX_MULTIPLEINDEX);
    ASSERT(wheelIndex <= MAX_WHEELINDEX);
    indexes_ = (uint32_t) (multipleIndex | (wheelIndex << 23));
  }

  void set(std::size_t sievingPrime,
           std::size_t multipleIndex,
           std::size_t wheelIndex)
  {
    ASSERT(multipleIndex <= MAX_MULTIPLEINDEX);
    ASSERT(wheelIndex <= MAX_WHEELINDEX);
    indexes_ = (uint32_t) (multipleIndex | (wheelIndex << 23));
    sievingPrime_ = (uint32_t) sievingPrime;
  }

  std::size_t getSievingPrime() const
  {
    return sievingPrime_;
  }

  std::size_t getMultipleIndex() const
  {
    return indexes_ & MAX_MULTIPLEINDEX;
  }

  std::size_t getWheelIndex() const
  {
    return indexes_ >> 23;
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
  bool empty() { return begin() == end(); }

  /// Get the sieving prime's bucket.
  /// For performance reasons we don't keep an array with all
  /// buckets. Instead we find the sieving prime's bucket by
  /// doing pointer arithmetic using the sieving prime's address.
  /// Since all buckets are aligned by sizeof(Bucket) we
  /// calculate the next address that is smaller than the sieving
  /// prime's address and that is aligned by sizeof(Bucket).
  /// That's the address of the sieving prime's bucket.
  ///
  static Bucket* get(SievingPrime* sievingPrime)
  {
    ASSERT(sievingPrime != nullptr);
    uintptr_t address = (uintptr_t) (void*) sievingPrime;
    // We need to adjust the address
    // in case the bucket is full.
    address -= 1;
    address -= address % sizeof(Bucket);
    return (Bucket*) (void*) address;
  }

  /// Returns true if the bucket is full with sieving primes
  /// (or if there is no bucket i.e. sievingPrime == nullptr).
  /// Each bucket's memory address is aligned by sizeof(Bucket)
  /// (which is a power of 2) in the MemoryPool. This allows
  /// us to quickly check if the bucket is full using the next
  /// sieving prime's address % sizeof(Bucket).
  ///
  static bool isFull(SievingPrime* sievingPrime)
  {
    uintptr_t address = (uintptr_t) (void*) sievingPrime;
    return address % sizeof(Bucket) == 0;
  }

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
