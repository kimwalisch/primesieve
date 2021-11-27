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
/// Copyright (C) 2020 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/MemoryPool.hpp>
#include <primesieve/config.hpp>
#include <primesieve/Bucket.hpp>
#include <primesieve/primesieve_error.hpp>

#include <algorithm>
#include <vector>

using std::size_t;

#if defined(__GNUC__) && __GNUC__ == 4

// gcc 4.9 does not implement std::align.
// Use the implementation from
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=57350#c11

using std::uintptr_t;

static inline void *align(size_t alignment, size_t size,
                          void *&ptr, size_t &space)
{
  uintptr_t pn = reinterpret_cast<uintptr_t>(ptr);
  uintptr_t aligned = (pn + alignment - 1) & -alignment;
  size_t padding = aligned - pn;
  if (space < size + padding) return nullptr;
  space -= padding;
  return ptr = reinterpret_cast<void *>(aligned);
}

#else

using std::align;

#endif

namespace primesieve {

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

void MemoryPool::allocateBuckets()
{
  if (memory_.empty())
    memory_.reserve(128);

  // Allocate a large chunk of memory
  size_t bytes = count_ * sizeof(Bucket);
  char* memory = new char[bytes];
  memory_.emplace_back(memory);
  void* ptr = memory;

  // Align pointer address to sizeof(Bucket)
  if (!align(sizeof(Bucket), sizeof(Bucket), ptr, bytes))
    throw primesieve_error("MemoryPool: failed to align memory!");

  initBuckets(ptr, bytes);
  increaseAllocCount();
}

void MemoryPool::initBuckets(void* memory, size_t bytes)
{
  Bucket* buckets = (Bucket*) memory;
  count_ = bytes / sizeof(Bucket);
  size_t i = 0;

  if ((size_t) buckets % sizeof(Bucket) != 0)
    throw primesieve_error("MemoryPool: failed to align memory!");

  if (count_ < 10)
    throw primesieve_error("MemoryPool: insufficient buckets allocated!");

  for (; i + 1 < count_; i++)
  {
    buckets[i].reset();
    buckets[i].setNext(&buckets[i + 1]);
  }

  buckets[i].reset();
  buckets[i].setNext(nullptr);
  stock_ = buckets;
}

void MemoryPool::increaseAllocCount()
{
  count_ += count_ / 8;
  size_t maxCount = config::MAX_ALLOC_BYTES / sizeof(Bucket);
  count_ = std::min(count_, maxCount);
}

} // namespace
