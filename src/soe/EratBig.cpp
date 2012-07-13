//
// Copyright (c) 2012 Kim Walisch, <kim.walisch@gmail.com>.
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

#include "EratBig.h"
#include "SieveOfEratosthenes.h"
#include "SieveOfEratosthenes-inline.h"
#include "WheelFactorization.h"
#include "imath.h"

#include <stdint.h>
#include <stdexcept>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <list>

namespace soe {

EratBig::EratBig(const SieveOfEratosthenes& soe) :
  Modulo210Wheel_t(soe),
  stock_(NULL),
  log2SieveSize_(ilog2(soe.getSieveSize())),
  moduloSieveSize_(soe.getSieveSize() - 1)
{
  // EratBig uses bitwise operations that require a power of 2 sieve size
  if (!isPowerOf2(soe.getSieveSize()))
    throw std::invalid_argument("EratBig: sieveSize must be a power of 2 (2^n).");
  setListsSize(soe);
  init();
}

EratBig::~EratBig()
{
  for (PointerIterator_t iter = pointers_.begin(); iter != pointers_.end(); ++iter)
    delete[] *iter;
}

void EratBig::setListsSize(const SieveOfEratosthenes& soe)
{
  uint_t maxSievingPrime  = soe.getSqrtStop() / SieveOfEratosthenes::NUMBERS_PER_BYTE;
  uint_t maxNextMultiple  = maxSievingPrime * getMaxFactor() + getMaxFactor();
  uint_t maxMultipleIndex = soe.getSieveSize() - 1 + maxNextMultiple;
  uint_t maxSegmentCount  = maxMultipleIndex >> log2SieveSize_;
  uint_t size = maxSegmentCount + 1;
  lists_.resize(size, NULL);
}

void EratBig::init()
{
  // initialize each bucket list with an empty bucket
  for (uint_t i = 0; i < lists_.size(); i++)
    pushBucket(lists_[i]);
}

void EratBig::moveBucket(Bucket& src, Bucket*& dest)
{
  src.setNext(dest);
  dest = &src;
}

/// Move an empty bucket from the stock_ to the front of list,
/// if the stock_ is empty new buckets are allocated first.
///
void EratBig::pushBucket(Bucket*& list)
{
  if (stock_ == NULL) {
    Bucket* more = new Bucket[BUCKETS_PER_ALLOC];
    stock_ = &more[0];
    pointers_.push_back(more);
    for(int i = 0; i < BUCKETS_PER_ALLOC - 1; i++)
      more[i].setNext(&more[i + 1]);
    more[BUCKETS_PER_ALLOC - 1].setNext(NULL);
  }
  Bucket* emptyBucket = stock_;
  stock_ = stock_->next();
  moveBucket(*emptyBucket, list);
}

/// Get the bucket list related to the segment of the next
/// multiple (multipleIndex) of a sievingPrime.
///
Bucket*& EratBig::getList(uint_t* multipleIndex)
{
  uint_t segment = *multipleIndex >> log2SieveSize_;
  *multipleIndex &= moduloSieveSize_;
  return lists_[segment];
}

/// Add a new sieving prime
/// @see addSievingPrime() in WheelFactorization.h
///
void EratBig::storeSievingPrime(uint_t sievingPrime, uint_t multipleIndex, uint_t wheelIndex)
{
  Bucket*& list = getList(&multipleIndex);
  if (!list->store(sievingPrime, multipleIndex, wheelIndex))
    pushBucket(list);
}

/// This algorithm is used to cross-off the multiples of big sieving
/// primes that have very few multiples per segment.
/// @see crossOffMultiples() in SieveOfEratosthenes.cpp
///
void EratBig::crossOff(uint8_t* sieve)
{
  // lists_[0] contains the buckets related to the current segment
  // i.e. its buckets contain all the sieving primes that have
  // multiples in the current segment
  Bucket*& list = lists_[0];

  while (list->hasNext() || !list->empty()) {
    Bucket* bucket = list;
    list = NULL;
    pushBucket(list);
    do {
      crossOff(sieve, *bucket);
      Bucket* processed = bucket;
      bucket = bucket->next();
      processed->reset();
      moveBucket(*processed, stock_);
    } while (bucket != NULL);
  }

  // lists_[0] has been processed, thus the list related to the
  // next segment lists_[1] moves to lists_[0] ...
  Bucket* tmp = list;
  std::copy(lists_.begin() + 1, lists_.end(), lists_.begin());
  lists_.back() = tmp;
}

/// Cross-off the next multiple of each sieving prime within the
/// current bucket. This algorithm uses a modulo 210 wheel that skips
/// multiples of 2, 3, 5 and 7.
/// This is an optimized implementation of Tomas Oliveira e Silva's
/// cache-friendly segmented sieve of Eratosthenes algorithm:
/// http://www.ieeta.pt/~tos/software/prime_sieve.html
///
void EratBig::crossOff(uint8_t* sieve, Bucket& bucket)
{
  WheelPrime* wPrime = bucket.begin();
  WheelPrime* end    = bucket.end();

  // 2 sieving primes are processed per loop iteration to break
  // the dependency chain and reduce pipeline stalls
  for (; wPrime + 2 <= end; wPrime += 2) {
    uint_t multipleIndex0 = wPrime[0].getMultipleIndex();
    uint_t wheelIndex0    = wPrime[0].getWheelIndex();
    uint_t sievingPrime0  = wPrime[0].getSievingPrime();
    uint_t multipleIndex1 = wPrime[1].getMultipleIndex();
    uint_t wheelIndex1    = wPrime[1].getWheelIndex();
    uint_t sievingPrime1  = wPrime[1].getSievingPrime();
    // cross-off the current multiple (unset corresponding bit) of
    // sievingPrime(0|1) and calculate the next multiple
    // @see unsetBit() in WheelFactorization.h
    unsetBit(sieve, sievingPrime0, &multipleIndex0, &wheelIndex0);
    unsetBit(sieve, sievingPrime1, &multipleIndex1, &wheelIndex1);
    Bucket*& list0 = getList(&multipleIndex0);
    Bucket*& list1 = getList(&multipleIndex1);
    // move sievingPrime(0|1) to the bucket list
    // related to its next multiple
    if (!list0->store(sievingPrime0, multipleIndex0, wheelIndex0))
      pushBucket(list0);
    if (!list1->store(sievingPrime1, multipleIndex1, wheelIndex1))
      pushBucket(list1);
  }

  if (wPrime != end) {
    uint_t multipleIndex = wPrime->getMultipleIndex();
    uint_t wheelIndex    = wPrime->getWheelIndex();
    uint_t sievingPrime  = wPrime->getSievingPrime();
    unsetBit(sieve, sievingPrime, &multipleIndex, &wheelIndex);
    Bucket*& list = getList(&multipleIndex);
    if (!list->store(sievingPrime, multipleIndex, wheelIndex))
      pushBucket(list);
  }
}

} // namespace soe
