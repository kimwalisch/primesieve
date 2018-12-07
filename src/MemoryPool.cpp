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

#include <algorithm>
#include <memory>
#include <vector>

using std::size_t;
using std::unique_ptr;

namespace primesieve {

void MemoryPool::reset(SievingPrime*& sievingPrime)
{
  if (!stock_)
    allocateBuckets();

  Bucket* bucket = stock_;
  stock_ = stock_->next();
  bucket->setNext(nullptr);
  sievingPrime = bucket->begin();
}

void MemoryPool::addBucket(SievingPrime*& sievingPrime)
{
  if (!stock_)
    allocateBuckets();

  Bucket* bucket = stock_;
  stock_ = stock_->next();

  Bucket* old = sievingPrime->getBucket();
  old->setEnd(sievingPrime);
  bucket->setNext(old);
  sievingPrime = bucket->begin();
}

void MemoryPool::freeBucket(Bucket* bucket)
{
  bucket->reset();
  bucket->setNext(stock_);
  stock_ = bucket;
}

void MemoryPool::allocateBuckets()
{
  if (memory_.empty())
    memory_.reserve(128);

  // allocate a large chunk of memory
  size_t bytes = sizeof(Bucket) * count_;
  bytes += sizeof(Bucket) - 1;
  char* memory = new char[bytes];
  memory_.emplace_back(unique_ptr<char[]>(memory));

  // convert raw memory into buckets
  void* ptr = memory;
  ptr = std::align(sizeof(Bucket), sizeof(Bucket), ptr, bytes);
  Bucket* buckets = (Bucket*) ptr;

  // initialize buckets
  for (size_t i = 0; i < count_; i++)
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
  size_t maxCount = config::MAX_ALLOC_BYTES / sizeof(Bucket);
  count_ = std::min(count_, maxCount);
}

} // namespace
