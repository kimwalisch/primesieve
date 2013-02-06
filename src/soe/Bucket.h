///
/// @file   Bucket.h
/// @brief  Buckets are storage containers for sieving primes.
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the New BSD License. See the
/// LICENSE file in the top level directory.
///

#ifndef BUCKET_PRIMESIEVE_H
#define BUCKET_PRIMESIEVE_H

#include "config.h"
#include "WheelFactorization.h"

namespace soe {

class Bucket;

/// The BucketCache class stores sieving primes faster than the Bucket
/// class. The bucket class requires two pointer accesses to store
/// a prime in EratBig whereas BucketCache uses only one.
///
class BucketCache {
public:
  WheelPrime* end()  const { return current_;}
  WheelPrime* last() const { return last_; }
  void set(Bucket&);
  /// store a WheelPrime in the associated Bucket object
  bool store(uint_t sievingPrime,
             uint_t multipleIndex,
             uint_t wheelIndex)
  {
    current_->set(sievingPrime, multipleIndex, wheelIndex);
    return current_++ != last_;
  }
private:
  WheelPrime* current_;
  WheelPrime* last_;
};

/// The Bucket data structure is used to store sieving primes.
/// @see http://www.ieeta.pt/~tos/software/prime_sieve.html
/// The Bucket class is designed as a singly linked list, once there
/// is no more space in the current Bucket a new Bucket node is
/// allocated.
///
class Bucket {
public:
  Bucket(const Bucket&) { reset(); }
  Bucket()              { reset(); }
  WheelPrime* begin()   { return &wheelPrimes_[0]; }
  WheelPrime* last()    { return &wheelPrimes_[config::BUCKETSIZE - 1]; }
  WheelPrime* end()     { return current_;}
  Bucket* next()        { return next_; }
  bool hasNext() const  { return next_ != NULL; }
  bool empty()          { return begin() == end(); }
  void reset()          { current_ = begin(); }
  void setFull()        { current_ = last() + 1; }
  void setNext(Bucket* next)
  {
    next_ = next;
  }
  void update(BucketCache& bucket)
  {
    current_ = bucket.end();
  }
  /// Store a WheelPrime in the bucket.
  /// @return false if the bucket is full else true.
  ///
  bool store(uint_t sievingPrime,
             uint_t multipleIndex,
             uint_t wheelIndex)
  {
    current_->set(sievingPrime, multipleIndex, wheelIndex);
    return current_++ != last();
  }
private:
  WheelPrime* current_;
  Bucket* next_;
  WheelPrime wheelPrimes_[config::BUCKETSIZE];
};


inline void BucketCache::set(Bucket& bucket)
{
  current_ = bucket.begin();
  last_    = bucket.last();
}

} // namespace soe

#endif
