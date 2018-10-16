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

#include "Bucket.hpp"

#include <stdint.h>
#include <memory>
#include <vector>

namespace primesieve {

class MemoryPool
{
public:
  void addBucket(Bucket*& list);
  void freeBucket(Bucket* b);
private:
  void allocateBuckets();
  void increaseAllocCount();
  /// List of empty buckets
  Bucket* stock_ = nullptr;
  /// Number of buckets to allocate
  uint64_t count_ = 128;
  /// Pointers of allocated buckets
  std::vector<std::unique_ptr<Bucket[]>> memory_;
};

} // namespace

#endif
