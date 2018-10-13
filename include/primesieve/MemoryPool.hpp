///
/// @file  MemoryPool.hpp
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

#ifndef MEMORYPOOL_HPP
#define MEMORYPOOL_HPP

#include "Bucket.hpp"
#include "config.hpp"

#include <stdint.h>
#include <algorithm>
#include <memory>
#include <vector>

#if !defined(__has_attribute)
  #define __has_attribute(x) 0
#endif

namespace primesieve {

class MemoryPool
{
public:
  /// addBucket() is called inside the main sieving loop in EratBig
  /// whenever the currently used bucket is full. Each bucket can
  /// store 1024 sieving primes so addBucket() is called very rarely
  /// and should hence not be inlined in order to avoid register
  /// spilling. However when using MSVC 2017 x64 and addBucket() is
  /// not inlined performance deteriorates by up to 30%.
  ///
  /// MSVC: addBucket() should be inlined.
  /// Other compilers: addBucket() should not be inlined.
  ///
  #if __has_attribute(noinline)
    __attribute__((noinline))
  #endif
  void addBucket(Bucket*& list)
  {
    // allocate new buckets
    if (!stock_)
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

      // put new buckets into stock
      stock_ = buckets;
  
      // next time allocate more buckets
      count_ += count_ / 8;
      uint64_t maxCount = config::MAX_ALLOC_BYTES / sizeof(Bucket);
      count_ = std::min(count_, maxCount);
    }

    // get first bucket
    Bucket& bucket = *stock_;
    stock_ = stock_->next();
    bucket.setNext(list);
    list = &bucket;
  }

  void freeBucket(Bucket* b)
  {
    Bucket& bucket = *b;
    bucket.reset();
    bucket.setNext(stock_);
    stock_ = &bucket;
  }

private:
  /// List of empty buckets
  Bucket* stock_ = nullptr;
  /// Number of buckets to allocate
  uint64_t count_ = 128;
  /// Pointers of allocated buckets
  std::vector<std::unique_ptr<Bucket[]>> memory_;
};

} // namespace

#endif
