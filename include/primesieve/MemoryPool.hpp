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
#include "config.hpp"

#include <stdint.h>
#include <vector>
#include <memory>

namespace primesieve {

class MemoryPool
{
public:
  void addBucket(Bucket*& dest);
  void freeBucket(Bucket* b);
private:
  void allocateBuckets();
  void increaseAllocCount();
  Bucket* getBucket();
  /// EratMedium::lists_.size() * 2 = 128
  /// so EratMedium will require only 1 allocation
  uint64_t count_ = 128;
  uint64_t maxCount_ = config::MAX_ALLOC_BYTES / sizeof(Bucket);
  /// List of empty buckets
  Bucket* stock_ = nullptr;
  /// Pointers of allocated buckets
  std::vector<std::unique_ptr<Bucket[]>> memory_;
};

} // namespace

#endif
