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

#include "EratBig.h"
#include "SieveOfEratosthenes.h"
#include "WheelFactorization.h"
#include "defs.h"
#include "bithacks.h"

#include <stdexcept>
#include <cstdlib>
#include <list>

EratBig::EratBig(const SieveOfEratosthenes& soe) :
  Modulo210Wheel(soe),
  lists_(NULL),
  stock_(NULL),
  index_(0),
  log2SieveSize_(floorLog2(soe.getSieveSize()))
{
  // EratBig uses bitwise operations that require a power of 2 sieve size
  if (!isPowerOf2(soe.getSieveSize()))
    throw std::invalid_argument(
        "EratBig: sieveSize must be a power of 2 (2^n).");
  this->setSize(soe);
  this->initBucketLists();
}

EratBig::~EratBig() {
  delete[] lists_;
  for (std::list<Bucket_t*>::iterator it = pointers_.begin();
      it != pointers_.end(); ++it)
    delete[] *it;
}

/**  
 * Set the size of the lists_ array.
 * @remark The size is a power of 2 value which allows use of fast
 *         bitwise operators in sieve(uint8_t*).
 */
void EratBig::setSize(const SieveOfEratosthenes& soe) {
  // MAX values in sieve(uint8_t*)
  uint32_t maxSievingPrime = soe.getSquareRoot() / (SieveOfEratosthenes::NUMBERS_PER_BYTE / 2);
  uint32_t maxWheelFactor  = wheel_[1].nextMultipleFactor;
  uint32_t maxSieveIndex   = (soe.getSieveSize() - 1) + maxSievingPrime * maxWheelFactor;
  uint32_t maxSegmentCount = maxSieveIndex / soe.getSieveSize();
  // 'maxSegmentCount + 1' is the smallest possible
  // size for the lists_ array
  size_ = nextHighestPowerOf2(maxSegmentCount + 1);
}

/**
 * Allocate the lists_ array and initialize each list
 * with an empty bucket.
 */
void EratBig::initBucketLists() {
  lists_ = new Bucket_t*[size_];
  for (uint32_t i = 0; i < size_; i++) {
    lists_[i] = NULL;
    this->pushBucket(i);
  }
}

/**
 * Add a prime number <= sqrt(n) for sieving to EratBig.
 */
void EratBig::addSievingPrime(uint32_t prime) {
  uint32_t sieveIndex;
  uint32_t wheelIndex;
  if (this->getWheelPrimeData(&prime, &sieveIndex, &wheelIndex) == true) {
    // indicates in how many segments the next multiple
    // of prime needs to be crossed-off
    uint32_t segmentCount = sieveIndex >> log2SieveSize_;
    sieveIndex &= (1U << log2SieveSize_) - 1;
    // calculate the list index related to the next multiple of prime
    uint32_t next = (index_ + segmentCount) & (size_ - 1);
    // add prime to the bucket list related to
    // its next multiple occurrence
    if (!lists_[next]->addWheelPrime(prime, sieveIndex, wheelIndex))
      this->pushBucket(next);
  }
}

/**
 * Add an empty bucket from the bucket stock_ to the
 * front of lists_[index], if the bucket stock_ is
 * empty new buckets are allocated first.
 */
void EratBig::pushBucket(uint32_t index) {
  if (stock_ == NULL) {
    Bucket_t* more = new Bucket_t[BUCKETS_PER_ALLOC];
    stock_ = &more[0];
    pointers_.push_back(more);
    for(int i = 0; i < BUCKETS_PER_ALLOC - 1; i++)
      more[i].setNext(&more[i + 1]);
    more[BUCKETS_PER_ALLOC - 1].setNext(NULL);
  }
  Bucket_t* bucket = stock_;
  stock_ = stock_->next();
  bucket->setNext(lists_[index]);
  lists_[index] = bucket;
}

/**
 * Implementation of Tomas Oliveira e Silva's cache-friendly segmented
 * sieve of Eratosthenes algorithm, see:
 * http://www.ieeta.pt/~tos/software/prime_sieve.html
 * My implementation uses a sieve array with 30 numbers per byte,
 * 8 bytes per sieving prime and a modulo 210 wheel that skips
 * multiples of 2, 3, 5 and 7.
 *
 * Removes the multiples of big sieving primes (i.e. with very few
 * multiple occurrences per segment) from the sieve array.
 * @see SieveOfEratosthenes::crossOffMultiples()
 */
void EratBig::sieve(uint8_t* sieve) {
  // get the bucket list related to the current segment,
  // the list contains the sieving primes with multiple occurence(s)
  // in the current segment
  Bucket_t*& list = lists_[index_];

  // process the buckets within list until all multiples of
  // its sieving primes have been crossed-off
  while (!list->isEmpty() || list->hasNext()) {
    Bucket_t* bucket = list;
    list = NULL;
    this->pushBucket(index_);

    // each loop iteration processes a bucket i.e. removes the
    // next multiple of its sieving primes
    do {
      const WheelPrime_t* wPrime = bucket->begin();
      const WheelPrime_t* end    = bucket->end();

      // iterate over the sieving primes within the current bucket
      // loop unrolled 2 times, optimized for x64 CPUs
      for (; &wPrime[2] <= end; wPrime += 2) {
        uint32_t sieveIndex0   = wPrime[0].getSieveIndex();
        uint32_t wheelIndex0   = wPrime[0].getWheelIndex();
        uint32_t sievingPrime0 = wPrime[0].getSievingPrime();
        uint32_t sieveIndex1   = wPrime[1].getSieveIndex();
        uint32_t wheelIndex1   = wPrime[1].getWheelIndex();
        uint32_t sievingPrime1 = wPrime[1].getSievingPrime();

        // cross-off the next multiple (unset corresponding bit) of the
        // current sieving primes within the sieve array
        sieve[sieveIndex0] &= wheel_[wheelIndex0].unsetBit;
        sieveIndex0        += wheel_[wheelIndex0].nextMultipleFactor * sievingPrime0;
        sieveIndex0        += wheel_[wheelIndex0].correct;
        wheelIndex0        += wheel_[wheelIndex0].next;
        sieve[sieveIndex1] &= wheel_[wheelIndex1].unsetBit;
        sieveIndex1        += wheel_[wheelIndex1].nextMultipleFactor * sievingPrime1;
        sieveIndex1        += wheel_[wheelIndex1].correct;
        wheelIndex1        += wheel_[wheelIndex1].next;

        uint32_t next0 = (index_ + (sieveIndex0 >> log2SieveSize_)) & (size_ - 1);
        sieveIndex0 &= (1U << log2SieveSize_) - 1;
        uint32_t next1 = (index_ + (sieveIndex1 >> log2SieveSize_)) & (size_ - 1);
        sieveIndex1 &= (1U << log2SieveSize_) - 1;

        // move the current sieving primes to the bucket list
        // related to their next multiple occurrence
        if (!lists_[next0]->addWheelPrime(sievingPrime0, sieveIndex0, wheelIndex0))
          this->pushBucket(next0);
        if (!lists_[next1]->addWheelPrime(sievingPrime1, sieveIndex1, wheelIndex1))
          this->pushBucket(next1);
      }

      // process the remaining sieving primes
      for (; wPrime < end; wPrime++) {
        uint32_t sieveIndex   = wPrime->getSieveIndex();
        uint32_t wheelIndex   = wPrime->getWheelIndex();
        uint32_t sievingPrime = wPrime->getSievingPrime();
        sieve[sieveIndex] &= wheel_[wheelIndex].unsetBit;
        sieveIndex += wheel_[wheelIndex].nextMultipleFactor * sievingPrime;
        sieveIndex += wheel_[wheelIndex].correct;
        wheelIndex += wheel_[wheelIndex].next;
        uint32_t next = (index_ + (sieveIndex >> log2SieveSize_)) & (size_ - 1);
        sieveIndex &= (1U << log2SieveSize_) - 1;
        if (!lists_[next]->addWheelPrime(sievingPrime, sieveIndex, wheelIndex))
          this->pushBucket(next);
      }

      // reset the processed bucket and move it to the bucket stock_
      Bucket_t* old = bucket;
      bucket = bucket->next();
      old->reset();
      old->setNext(stock_);
      stock_ = old;
    }
    while (bucket != NULL);
  }

  // increase the list index_ for the next segment
  index_ = (index_ + 1) & (size_ - 1);
}
