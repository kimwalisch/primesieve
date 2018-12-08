///
/// @file   Bucket.hpp
/// @brief  A bucket is a container for sieving primes.
///         The Bucket class is designed as a singly linked list,
///         once there is no more space in the current Bucket
///         a new Bucket is allocated.
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
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

class Bucket;

/// Each SievingPrime object contains a sieving prime and the
/// position of its next multiple inside the sieve array i.e.
/// multipleIndex and a wheelIndex. In order to reduce the memory
/// usage the multipleIndex and wheelIndex are packed into a
/// single 32-bit variable.
///
class SievingPrime
{
public:
  enum
  {
    MAX_MULTIPLEINDEX = (1 << 23) - 1,
    MAX_WHEELINDEX    = (1 << (32 - 23)) - 1
  };

  SievingPrime() = default;
  Bucket* getBucket() const;
  bool empty() const;
  bool isBucketFull() const;

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

  bool set(uint64_t sievingPrime,
           uint64_t multipleIndex,
           uint64_t wheelIndex)
  {
    set(multipleIndex, wheelIndex);
    sievingPrime_ = (uint32_t) sievingPrime;
    return !isBucketFull();
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
  bool hasNext() const  { return next_ != nullptr; }
  bool empty()          { return begin() == end(); }
  void setNext(Bucket* next) { next_ = next; }
  void setEnd(SievingPrime* end) { end_ = end; }
  void reset() { end_ = begin(); }

private:
  SievingPrime* end_;
  Bucket* next_;
  SievingPrime sievingPrimes_[(config::BUCKET_BYTES - sizeof(SievingPrime*) - sizeof(Bucket*)) / sizeof(SievingPrime)];
};

static_assert(isPow2(sizeof(Bucket)), "sizeof(Bucket) must be a power of 2");

/// Get the current sieving prime's bucket.
/// For performance reasons we don't keep an array with all
/// buckets. Instead we find the sieving prime's bucket by
/// doing pointer arithmetic using the sieving prime's
/// address. Since all buckets are aligned by sizeof(Bucket)
/// we calculate the next address that is smaller than the
/// sieving prime's address and that is aligned by
/// sizeof(Bucket). That's the address of the sieving prime's
/// bucket.
///
inline Bucket* SievingPrime::getBucket() const
{
  std::size_t address = (std::size_t) this;
  // We need to adjust the address
  // in case the bucket is full
  address -= 1;
  address -= address % sizeof(Bucket);
  return (Bucket*) address;
}

/// Returns true if the sieving prime's bucket is full.
/// Since each bucket's memory is aligned by sizeof(Bucket)
/// we can compute the position of the current sieving
/// prime using address % sizeof(Bucket).
///
inline bool SievingPrime::isBucketFull() const
{
  std::size_t address = (std::size_t) this;
  return (address + sizeof(SievingPrime)) % sizeof(Bucket) == 0;
}

/// Returns true if the sieving prime's bucket is empty
/// and if that bucket does not have a pointer to other
/// buckets full with sieving primes.
///
inline bool SievingPrime::empty() const
{
  std::size_t address = (std::size_t) this;
  std::size_t begin = sizeof(SievingPrime*) + sizeof(Bucket*);
  bool isEmpty = (address % sizeof(Bucket)) == begin;
  return isEmpty && !getBucket()->hasNext();
}

} // namespace

#endif
