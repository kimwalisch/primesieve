//
// Copyright (c) 2011 Kim Walisch, <kim.walisch@gmail.com>.
// All rights reserved.
//
// This file is part of primesieve.
// Homepage: http://primesieve.googlecode.com
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//   * Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above
//     copyright notice, this list of conditions and the following
//     disclaimer in the documentation and/or other materials provided
//     with the distribution.
//   * Neither the name of the author nor the names of its
//     contributors may be used to endorse or promote products derived
//     from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef ERATBASE_H
#define ERATBASE_H

#include "WheelFactorization.h"
#include "defs.h"

#include <cassert>
#include <cstdlib>
#include <list>

class SieveOfEratosthenes;

/**
 * EratBase is an abstract class used by EratSmall and EratMedium to
 * initialize and store sieving primes.
 */
template<class T_Wheel>
class EratBase: protected T_Wheel {
public:
  uint32_t getLimit() const {
    return limit_;
  }
  /** Add a prime number for sieving to EratBase. */
  void addSievingPrime(uint32_t prime) {
    assert(prime <= limit_);
    uint32_t multipleIndex;
    uint32_t wheelIndex;
    if (this->getWheelPrimeData(&prime, &multipleIndex, &wheelIndex)
        == true) {
      if (!buckets_.back().addWheelPrime(prime, multipleIndex, wheelIndex)) {
        // the current bucket is full, add a new one
        buckets_.push_back(Bucket());
      }
    }
  }
protected:
  typedef std::list<Bucket> BucketList_t;
  /** Upper bound for sieving primes within EratBase. */
  uint32_t limit_;
  /** List of buckets, holds the sieving primes. */
  BucketList_t buckets_;
  EratBase(const SieveOfEratosthenes& soe) : T_Wheel(soe) {
    // initialize with an empty bucket
    buckets_.push_back(Bucket());
  }
  void setLimit(uint32_t limit) {
    limit_ = limit;
  }
};

#endif /* ERATBASE_H */
