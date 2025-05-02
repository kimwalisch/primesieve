///
/// @file  MemoryPool.hpp
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef MEMORYPOOL_HPP
#define MEMORYPOOL_HPP

#include <primesieve/macros.hpp>
#include <primesieve/Vector.hpp>

namespace primesieve {

class SievingPrime;
class Bucket;

class MemoryPool
{
public:
  NOINLINE void addBucket(SievingPrime*& sievingPrime);
  void freeBucket(Bucket* bucket);

private:
  void updateAllocCount();
  void allocateBuckets();
  void initBuckets(void* alignedPtr);
  /// List of empty buckets
  Bucket* stock_ = nullptr;
  /// Number of buckets to allocate
  std::size_t count_ = 0;
  /// Pointers of allocated buckets
  Vector<Vector<char>> memory_;
};

} // namespace

#endif
