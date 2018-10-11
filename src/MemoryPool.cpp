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

void MemoryPool::allocateBuckets()
{
  if (memory_.empty())
    memory_.reserve(128);

  using std::unique_ptr;
  Bucket* buckets = new Bucket[count_];
  memory_.emplace_back(unique_ptr<Bucket[]>(buckets));

  for (uint64_t i = 0; i < count_; i++)
  {
    Bucket* next = nullptr;
    if (i + 1 < count_)
      next = &buckets[i + 1];

    buckets[i].reset();
    buckets[i].setNext(next);
  }

  stock_ = buckets;
  increaseAllocCount();
}

void MemoryPool::increaseAllocCount()
{
  count_ += count_ / 8;
  if (count_ > maxCount_)
    count_ = maxCount_;
}

Bucket* MemoryPool::getBucket()
{
  if (!stock_)
    allocateBuckets();

  Bucket* bucket = stock_;
  stock_ = stock_->next();
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
  bucket.reset();
  bucket.setNext(stock_);
  stock_ = &bucket;
}

} // namespace
