///
/// @file  MemoryPool.hpp
///
/// Copyright (C) 2019 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef MEMORYPOOL_HPP
#define MEMORYPOOL_HPP

#include "Bucket.hpp"
#include "noinline.hpp"

#include <cassert>
#include <memory>
#include <vector>

namespace primesieve {

class MemoryPool
{
public:
  NOINLINE void addBucket(SievingPrime*& sievingPrime);
  void freeBucket(Bucket* bucket);

  /// Get the sieving prime's bucket.
  /// For performance reasons we don't keep an array with all
  /// buckets. Instead we find the sieving prime's bucket by
  /// doing pointer arithmetic using the sieving prime's address.
  /// Since all buckets are aligned by sizeof(Bucket) we
  /// calculate the next address that is smaller than the sieving
  /// prime's address and that is aligned by sizeof(Bucket).
  /// That's the address of the sieving prime's bucket.
  ///
  static Bucket* getBucket(SievingPrime* sievingPrime)
  {
    assert(sievingPrime != nullptr);
    std::size_t address = (std::size_t) sievingPrime;
    // We need to adjust the address
    // in case the bucket is full.
    address -= 1;
    address -= address % sizeof(Bucket);
    return (Bucket*) address;
  }

  /// Returns true if the sieving prime's bucket is full (or if
  /// there is no bucket at all i.e. sievingPrime == nullptr).
  /// Since each bucket's memory is aligned by sizeof(Bucket) we
  /// can compute the position of the current sieving prime
  /// using address % sizeof(Bucket).
  ///
  static bool isFullBucket(SievingPrime* sievingPrime)
  {
    auto address = (std::size_t) sievingPrime;
    return address % sizeof(Bucket) == 0;
  }

private:
  void allocateBuckets();
  void initBuckets(void* memory, std::size_t bytes);
  void increaseAllocCount();
  /// List of empty buckets
  Bucket* stock_ = nullptr;
  /// Number of buckets to allocate
  std::size_t count_ = 64;
  /// Pointers of allocated buckets
  std::vector<std::unique_ptr<char[]>> memory_;
};

} // namespace

#endif
