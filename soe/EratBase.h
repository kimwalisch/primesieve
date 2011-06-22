/*
 * EratBase.h -- This file is part of primesieve
 *
 * Copyright (C) 2011 Kim Walisch, <kim.walisch@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef ERATBASE_H
#define ERATBASE_H

#include "WheelFactorization.h"
#include "defs.h"

#include <stdexcept>
#include <cassert>
#include <cstdlib>

class SieveOfEratosthenes;

/**
 * EratBase is an abstract class used by EratSmall and EratMedium to
 * initialize an store sieving primes.
 */
template<class T_ModuloWheel>
class EratBase: protected T_ModuloWheel {
public:
  uint32_t getLimit() const {
    return limit_;
  }
  /** Add a prime number for sieving to EratBase. */
  void addSievingPrime(uint32_t prime, uint64_t segmentLow) {
    assert(prime <= limit_);
    uint32_t sieveIndex;
    uint32_t wheelIndex;
    if (this->setWheelPrime(segmentLow, &prime, &sieveIndex, &wheelIndex)
        == true) {
      if (!bucketList_->addWheelPrime(prime, sieveIndex, wheelIndex)) {
        Bucket_t* bucket = new Bucket_t;
        bucket->setNext(bucketList_);
        bucketList_ = bucket;
      }
    }
  }
protected:
  typedef Bucket<defs::ERATBASE_BUCKETSIZE> Bucket_t;
  /** Upper bound for sieving primes within bucketList_. */
  const uint32_t limit_;
  /** Singly linked list of buckets, holds the sieving primes. */
  Bucket_t* bucketList_;
  EratBase(uint32_t limit, const SieveOfEratosthenes* soe) :
    T_ModuloWheel(soe), limit_(limit), bucketList_(NULL) {
    if (limit > isqrt(this->stopNumber_))
      throw std::logic_error("EratBase: limit must be <= sqrt(stopNumber).");
    // initialize the bucket list with an empty bucket
    bucketList_ = new Bucket_t;
    bucketList_->setNext(NULL);
  }
  ~EratBase() {
    while (bucketList_ != NULL) {
      Bucket_t* old = bucketList_;
      bucketList_ = bucketList_->next;
      delete old;
    }
  }
};

#endif /* ERATBASE_H */
