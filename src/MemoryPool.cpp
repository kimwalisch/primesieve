///
/// @file  MemoryPool.cpp
///        EratMedium and EratBig may use millions of buckets for
///        storing the sieving primes that are required to cross off
///        multiples. As many memory allocations/deallocations are
///        bad for performance the MemoryPool initially allocates a
///        large number of buckets (using a single memory allocation)
///        and puts the buckets into its stock. The MemoryPool can
///        then serve buckets to EratMedium and EratBig without
///        doing any memory allocation as long as the MemoryPool's
///        stock is not empty.
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/MemoryPool.hpp>
#include <primesieve/config.hpp>
#include <primesieve/Bucket.hpp>

#include <stdint.h>
#include <algorithm>
#include <memory>
#include <vector>

namespace primesieve {

void MemoryPool::addBucket(Bucket*& list)
{
  if (!stock_)
    allocateBuckets();

  // get first bucket
  Bucket& bucket = *stock_;
  stock_ = stock_->next();
  bucket.setNext(list);
  list = &bucket;
}

void MemoryPool::freeBucket(Bucket* b)
{
  Bucket& bucket = *b;
  bucket.reset();
  bucket.setNext(stock_);
  stock_ = &bucket;
}

void MemoryPool::allocateBuckets()
{
  if (memory_.empty())
    memory_.reserve(128);

  using std::unique_ptr;
  Bucket* buckets = new Bucket[count_];
  memory_.emplace_back(unique_ptr<Bucket[]>(buckets));

  // initialize buckets
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
  uint64_t maxCount = config::MAX_ALLOC_BYTES / sizeof(Bucket);
  count_ = std::min(count_, maxCount);
}

} // namespace
