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

namespace primesieve {

class MemoryPool
{
public:
  void addBucket(Bucket*& list)
  {
    // allocate new buckets
    if (!stock_)
    {
      if (memory_.empty())
        memory_.reserve(128);

      Bucket* buckets = new Bucket[count_];
      memory_.emplace_back(std::unique_ptr<Bucket[]>(buckets));

      // init buckets
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
      count_ += count_ / 8;
      count_ = std::min(count_, maxCount_);
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
