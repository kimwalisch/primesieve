///
/// @file  MemoryPool.hpp
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef MEMORYPOOL_HPP
#define MEMORYPOOL_HPP

#include <vector>
#include <memory>

namespace primesieve {

class Bucket;
class SievingPrime;

class MemoryPool
{
public:
  void reset(SievingPrime*& sievingPrime);
  void addBucket(SievingPrime*& sievingPrime);
  void freeBucket(Bucket* bucket);
private:
  void allocateBuckets();
  void initBuckets(Bucket* buckets);
  void increaseAllocCount();
  /// List of empty buckets
  Bucket* stock_ = nullptr;
  /// Number of buckets to allocate
  std::size_t count_ = 128;
  /// Pointers of allocated buckets
  std::vector<std::unique_ptr<char[]>> memory_;
};

} // namespace

#endif
