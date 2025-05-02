///
/// @file   MemoryPool.cpp
/// @brief  EratMedium and EratBig may use millions of buckets for
///         storing the sieving primes that are required to cross off
///         multiples. As many memory allocations/deallocations are
///         bad for performance the MemoryPool initially allocates a
///         large number of buckets (using a single memory allocation)
///         and puts the buckets into its stock. The MemoryPool can
///         then serve buckets to EratMedium and EratBig without
///         doing any memory allocation as long as the MemoryPool's
///         stock is not empty.
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include "MemoryPool.hpp"
#include "Bucket.hpp"

#include <primesieve/config.hpp>
#include <primesieve/macros.hpp>
#include <primesieve/Vector.hpp>
#include <primesieve/primesieve_error.hpp>

#include <algorithm>
#include <memory>

namespace primesieve {

void MemoryPool::updateAllocCount()
{
  std::size_t allocationNr = memory_.size() + 1;

  if (allocationNr == 1)
  {
    // Default number of buckets for the 1st allocation.
    // EratMedium requires exactly 73 buckets for small sieving limits.
    // EratMedium requires one bucket for each of its 64 bucket lists
    // and an additional 8 buckets whilst sieving. We likely also waste
    // 1 bucket in order to align all our buckets' memory addresses to
    // power of 2 boundaries: &bucket % sizeof(Bucket) == 0.
    count_ = 73;

    // 64 MemoryPool allocations (per thread)
    // are enough to sieve up to 9e17.
    memory_.reserve(64);
  }
  else if (allocationNr == 2)
  {
    // The 1st allocation allocates a fairly large number of buckets
    // (73) to initialize the EratMedium.cpp algorithm. For the 2nd
    // allocation we set the number of buckets to a smaller value
    // (count_ / 4) to reduce the memory usage.
    std::size_t minBuckets = 16;
    count_ = std::max(minBuckets, count_ / 4);
  }
  else
  {
    // From the 3rd allocation onwards, we slowly increase the number
    // of buckets to allocate. Increasing the number of buckets
    // reduces the number of allocations, but on the other hand also
    // adds some memory usage overhead.
    count_ += count_ / 8;
    std::size_t maxCount = config::MAX_ALLOC_BYTES / sizeof(Bucket);
    count_ = std::min(count_, maxCount);
  }
}

void MemoryPool::allocateBuckets()
{
  updateAllocCount();

  // Allocate a large chunk of memory
  std::size_t bytes = count_ * sizeof(Bucket);
  memory_.emplace_back(bytes);
  void* ptr = (void*) memory_.back().data();

  // Align pointer address to sizeof(Bucket)
  if_unlikely(!std::align(sizeof(Bucket), sizeof(Bucket), ptr, bytes))
    throw primesieve_error("MemoryPool: failed to align memory!");

  count_ = bytes / sizeof(Bucket);
  initBuckets(ptr);
}

void MemoryPool::initBuckets(void* alignedPtr)
{
  Bucket* buckets = (Bucket*) alignedPtr;

  if_unlikely((std::size_t) buckets % sizeof(Bucket) != 0)
    throw primesieve_error("MemoryPool: failed to align memory!");
  if_unlikely(count_ < 10)
    throw primesieve_error("MemoryPool: insufficient buckets allocated!");

  for (std::size_t i = 0; i < count_ - 1; i++)
  {
    buckets[i].reset();
    buckets[i].setNext(&buckets[i + 1]);
  }

  buckets[count_ - 1].reset();
  buckets[count_ - 1].setNext(nullptr);
  stock_ = buckets;
}

void MemoryPool::addBucket(SievingPrime*& sievingPrime)
{
  if (!stock_)
    allocateBuckets();

  Bucket* bucket = stock_;
  stock_ = stock_->next();
  bucket->setNext(nullptr);

  // In case we add a bucket to the front of a
  // non empty bucket list we need to set the
  // next pointer of the new bucket to the bucket
  // that was previously at the front of the list.
  if (sievingPrime)
  {
    Bucket* old = Bucket::get(sievingPrime);
    old->setEnd(sievingPrime);
    bucket->setNext(old);
  }

  sievingPrime = bucket->begin();
}

void MemoryPool::freeBucket(Bucket* bucket)
{
  bucket->reset();
  bucket->setNext(stock_);
  stock_ = bucket;
}

} // namespace
