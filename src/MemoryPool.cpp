///
/// @file  MemoryPool.cpp
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/MemoryPool.hpp>

#include <stdint.h>
#include <memory>
#include <vector>

namespace primesieve {

void MemoryPool::setAllocCount(uint64_t count)
{
  count_ = count;
}

Bucket* MemoryPool::getBucket()
{
  // allocate new buckets
  if (!stock_)
  {
    Bucket* buckets = new Bucket[count_];
    memory_.emplace_back(std::unique_ptr<Bucket[]>(buckets));
    for (uint64_t i = 0; i < count_ - 1; i++)
      buckets[i].setNext(&buckets[i + 1]);
    buckets[count_ - 1].setNext(nullptr);
    stock_ = buckets;
  }

  Bucket* bucket = stock_;
  stock_ = stock_->next();
  bucket->reset();
  return bucket;
}

void MemoryPool::addBucket(Bucket*& dest)
{
  Bucket& bucket = *getBucket();
  bucket.setNext(dest);
  dest = &bucket;
}

void MemoryPool::freeBucket(Bucket* b)
{
  Bucket& bucket = *b;
  bucket.setNext(stock_);
  stock_ = &bucket;
}

} // namespace
