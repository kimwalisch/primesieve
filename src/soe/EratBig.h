//
// Copyright (c) 2011 Kim Walisch, <kim.walisch@gmail.com>.
// All rights reserved.
//
// This file is part of primesieve.
// Visit: http://primesieve.googlecode.com
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

#ifndef ERATBIG_H
#define ERATBIG_H

#include "WheelFactorization.h"
#include "defs.h"

#include <list>

class SieveOfEratosthenes;

/**
 * EratBig is my implementation of Tomas Oliveira e Silva's
 * cache-friendly segmented sieve of Eratosthenes algorithm, see:
 * http://www.ieeta.pt/~tos/software/prime_sieve.html
 * My implementation uses a sieve array with 30 numbers per byte,
 * 8 bytes per sieving prime and a modulo 210 wheel that skips
 * multiples of 2, 3, 5 and 7. Furthermore each sieving prime
 * (WheelPrime_t) has at most one multiple occurrence per segment
 * which allows to unroll the main loop in sieve(uint8_t*).
 */
class EratBig: protected Modulo210Wheel {
public:
  EratBig(const SieveOfEratosthenes&);
  ~EratBig();
  void addSievingPrime(uint32_t);
  void sieve(uint8_t*);
private:
  typedef WheelPrime_1 WheelPrime_t;
  typedef Bucket<WheelPrime_t, defs::ERATBIG_BUCKETSIZE> Bucket_t;
  enum { 
    BUCKETS_PER_CREATE = defs::ERATBIG_MEMORY_PER_ALLOC / sizeof(Bucket_t)
  };
  /** Current count of sieving primes within EratBig. */
  uint32_t primeCount_;
  /** log2 of SieveOfEratosthenes::sieveSize_. */
  const uint32_t log2SieveSize_;
  /** Size of bucketLists_. */
  uint32_t size_;
  /**
   * bucketLists_[index_]   holds the sieving primes with multiple occurrences in the current segment,
   * bucketLists_[index_+1] holds the sieving primes with multiple occurrences in the next segment,
   * ...
   */
  uint32_t index_;
  /** Array of bucket lists, holds the sieving primes. */
  Bucket_t** bucketLists_;
  /** List of empty buckets. */
  Bucket_t* bucketStock_;
  /** Pointers of the allocated buckets. */
  std::list<Bucket_t*> bucketPointers_;
  void setSize(const SieveOfEratosthenes&);
  void initBucketLists();
  void pushBucket(uint32_t);
};

#endif /* ERATBIG_H */
