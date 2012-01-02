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
#include "WheelFactorization.h"
#include "defs.h"
#include "bithacks.h"

#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <list>

EratBig::EratBig(const SieveOfEratosthenes& soe) :
  Modulo210Wheel_t(soe),
  lists_(NULL),
  stock_(NULL),
  log2SieveSize_(floorLog2(soe.getSieveSize())),
  moduloSieveSize_(soe.getSieveSize() - 1)
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
  for (std::list<Bucket*>::iterator it = pointers_.begin();
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
  uint32_t maxSievingPrime   = soe.getSquareRoot() / SieveOfEratosthenes::NUMBERS_PER_BYTE;
  uint32_t maxWheelFactor    = wheel(1)->nextMultipleFactor;
  uint32_t maxMultipleOffset = maxSievingPrime * maxWheelFactor + maxWheelFactor;
  uint32_t maxMultipleIndex  = (soe.getSieveSize() - 1) + maxMultipleOffset;
  uint32_t maxSegmentCount   = maxMultipleIndex / soe.getSieveSize();
  // 'maxSegmentCount + 1' is the smallest possible
  // size for the lists_ array
  size_ = nextHighestPowerOf2(maxSegmentCount + 1);
  moduloListsSize_ = size_ - 1;
}

/**
 * Allocate the lists_ array and initialize each list
 * with an empty bucket.
 */
void EratBig::initBucketLists() {
  lists_ = new Bucket*[size_];
  for (uint32_t i = 0; i < size_; i++) {
    lists_[i] = NULL;
    this->pushBucket(i);
  }
}

/**
 * Add a prime number <= sqrt(n) for sieving to EratBig.
 */
void EratBig::addSievingPrime(uint32_t prime) {
  uint32_t multipleIndex;
  uint32_t wheelIndex;
  if (this->getWheelPrimeData(&prime, &multipleIndex, &wheelIndex) == true) {
    // indicates in how many segments the next multiple
    // of prime needs to be crossed-off
    uint32_t segmentCount = multipleIndex >> log2SieveSize_;
    // index for the SieveOfEratosthenes::sieve_ array
    multipleIndex &= moduloSieveSize_;
    // calculate the list index related to the next multiple of prime
    uint32_t next = segmentCount & moduloListsSize_;
    // add prime to the bucket list related
    // to its next multiple occurrence
    if (!lists_[next]->addWheelPrime(prime, multipleIndex, wheelIndex))
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
    Bucket* more = new Bucket[BUCKETS_PER_ALLOC];
    stock_ = &more[0];
    pointers_.push_back(more);
    for(int i = 0; i < BUCKETS_PER_ALLOC - 1; i++)
      more[i].setNext(&more[i + 1]);
    more[BUCKETS_PER_ALLOC - 1].setNext(NULL);
  }
  Bucket* bucket = stock_;
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
void EratBig::sieve(uint8_t* sieve)
{
  // lists_[0] contains the list of buckets related to the current
  // segment i.e. its buckets contain all the sieving primes that have
  // multiple occurrence(s) in the current segment
  while (!lists_[0]->isEmpty() || lists_[0]->hasNext()) {
    Bucket* bucket = lists_[0];
    lists_[0] = NULL;
    this->pushBucket(0);

    // each loop iteration processes a bucket i.e. removes
    // the next multiple of its sieving primes
    do {
      const WheelPrime* wPrime = bucket->begin();
      const WheelPrime* end    = bucket->end();

      // For out-of-order CPUs this algorithm can be sped up by
      // processing 2 sieving primes per loop iteration, this breaks
      // the dependency chain and reduces pipeline stalls
      for (; wPrime + 2 <= end; wPrime += 2) {
        uint32_t multipleIndex0 = wPrime[0].getMultipleIndex();
        uint32_t wheelIndex0    = wPrime[0].getWheelIndex();
        uint32_t sievingPrime0  = wPrime[0].getSievingPrime();
        uint32_t multipleIndex1 = wPrime[1].getMultipleIndex();
        uint32_t wheelIndex1    = wPrime[1].getWheelIndex();
        uint32_t sievingPrime1  = wPrime[1].getSievingPrime();
        // cross-off the next multiple (unset corresponding bit) of
        // the current sieving primes within the sieve array
        sieve[multipleIndex0] &= wheel(wheelIndex0)->unsetBit;
        multipleIndex0        += wheel(wheelIndex0)->nextMultipleFactor * sievingPrime0;
        multipleIndex0        += wheel(wheelIndex0)->correct;
        wheelIndex0           += wheel(wheelIndex0)->next;
        sieve[multipleIndex1] &= wheel(wheelIndex1)->unsetBit;
        multipleIndex1        += wheel(wheelIndex1)->nextMultipleFactor * sievingPrime1;
        multipleIndex1        += wheel(wheelIndex1)->correct;
        wheelIndex1           += wheel(wheelIndex1)->next;
        uint32_t next0 = (multipleIndex0 >> log2SieveSize_) & moduloListsSize_;
        multipleIndex0 &= moduloSieveSize_;
        uint32_t next1 = (multipleIndex1 >> log2SieveSize_) & moduloListsSize_;
        multipleIndex1 &= moduloSieveSize_;
        // move the current sieving primes to the bucket list
        // related to their next multiple occurrence
        if (!lists_[next0]->addWheelPrime(sievingPrime0, multipleIndex0, wheelIndex0))
          this->pushBucket(next0);
        if (!lists_[next1]->addWheelPrime(sievingPrime1, multipleIndex1, wheelIndex1))
          this->pushBucket(next1);
      }

      // process the remaining sieving prime
      if (wPrime != end) {
        uint32_t multipleIndex = wPrime->getMultipleIndex();
        uint32_t wheelIndex    = wPrime->getWheelIndex();
        uint32_t sievingPrime  = wPrime->getSievingPrime();
        sieve[multipleIndex] &= wheel(wheelIndex)->unsetBit;
        multipleIndex        += wheel(wheelIndex)->nextMultipleFactor * sievingPrime;
        multipleIndex        += wheel(wheelIndex)->correct;
        wheelIndex           += wheel(wheelIndex)->next;
        uint32_t next = (multipleIndex >> log2SieveSize_) & moduloListsSize_;
        multipleIndex &= moduloSieveSize_;
        if (!lists_[next]->addWheelPrime(sievingPrime, multipleIndex, wheelIndex))
          this->pushBucket(next);
      }

      // reset the processed bucket and move it to the bucket stock_
      Bucket* old = bucket;
      bucket = bucket->next();
      old->reset();
      old->setNext(stock_);
      stock_ = old;
    }
    while (bucket != NULL);
  }

  // lists_[0] has been processed, thus the list related to
  // the next segment lists_[1] moves to lists_[0]
  Bucket* tmp = lists_[0];
  std::memmove(lists_, &lists_[1], (size_ - 1) * sizeof(Bucket*));
  lists_[size_ - 1] = tmp;
}
