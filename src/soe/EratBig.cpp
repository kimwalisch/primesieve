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
#include <cassert>
#include <list>

EratBig::EratBig(const SieveOfEratosthenes& soe) : Modulo210Wheel(soe),
    primeCount_(0), log2SieveSize_(floorLog2(soe.getSieveSize())),
    index_(0), bucketLists_(NULL), bucketStock_(NULL) {
  // EratBig uses bitwise operations that require a power of 2 sieve size
  if (!isPowerOf2(soe.getSieveSize()))
    throw std::invalid_argument(
        "EratBig: sieveSize must be a power of 2 (2^n).");
  this->setSize(soe);
  this->initBucketLists();
}

EratBig::~EratBig() {
  delete[] bucketLists_;
  for (std::list<Bucket_t*>::iterator it = bucketPointers_.begin();
      it != bucketPointers_.end(); it++)
    delete[] *it;
}

/**  
 * Set the size of the bucketLists_ array.
 * @remark The size is a power of 2 value which allows use of fast
 *         bitwise operators in sieve(uint8_t*).
 */
void EratBig::setSize(const SieveOfEratosthenes& soe) {
  uint32_t sqrtStop  = soe.getSquareRoot();
  uint32_t sieveSize = soe.getSieveSize();
  // MAX values in sieve(uint8_t*)
  uint32_t maxSievingPrime = sqrtStop / (SieveOfEratosthenes::NUMBERS_PER_BYTE / 2);
  uint32_t maxWheelFactor  = wheel_[1].nextMultipleFactor;
  uint32_t maxSieveIndex   = (sieveSize - 1) + maxSievingPrime * maxWheelFactor;
  uint32_t maxSegmentCount = maxSieveIndex / sieveSize;
  // 'maxSegmentCount + 1' is the smallest possible size for the
  // bucketLists_ array
  size_ = nextHighestPowerOf2(maxSegmentCount + 1);
}

/**
 * Allocate the bucketLists_ array and initialize each list with an
 * empty bucket.
 */
void EratBig::initBucketLists() {
  assert(size_ > 0);
  bucketLists_ = new Bucket_t*[size_];
  for (uint32_t i = 0; i < size_; i++) {
    bucketLists_[i] = NULL;
    this->pushBucket(i);
  }
}

/**
 * Add a prime number for sieving to EratBig.
 * @pre prime >= sieveSize * 15
 */
void EratBig::addSievingPrime(uint32_t prime) {
  uint32_t sieveIndex;
  uint32_t wheelIndex;
  if (this->getWheelPrimeData(&prime, &sieveIndex, &wheelIndex) == true) {
    primeCount_++;
    // indicates in how many segments the next multiple of prime
    // needs to be crossed off
    uint32_t segmentCount = sieveIndex >> log2SieveSize_;
    // calculate the bucket list related to the next multiple of prime
    sieveIndex &= (1U << log2SieveSize_) - 1;
    uint32_t nextIndex = (index_ + segmentCount) & (size_ - 1);
    // add the sieving prime to the appropriate bucket list
    if (!bucketLists_[nextIndex]->addWheelPrime(prime, sieveIndex, wheelIndex))
      this->pushBucket(nextIndex);
  }
}

/**
 * Add an empty bucket from the bucketStock_ to the front of
 * bucketLists_[index], if the bucketStock_ is empty new buckets are
 * allocated first.
 */
void EratBig::pushBucket(uint32_t index) {
  if (bucketStock_ == NULL) {
    Bucket_t* more = new Bucket_t[BUCKETS_PER_CREATE];
    for(int i = 0; i < BUCKETS_PER_CREATE - 1; i++)
      more[i].setNext(&more[i+1]);
    more[BUCKETS_PER_CREATE-1].setNext(NULL);
    bucketStock_ = &more[0];
    bucketPointers_.push_back(more);
  }
  assert(bucketStock_ != NULL);
  Bucket_t* bucket = bucketStock_;
  bucketStock_ = bucketStock_->next;
  bucket->next = bucketLists_[index];
  bucketLists_[index] = bucket;
}

/**
 * Implementation of Tomas Oliveira e Silva's cache-friendly segmented
 * sieve of Eratosthenes, the algorithm is described at:
 * http://www.ieeta.pt/~tos/software/prime_sieve.html
 * My implementation uses 30 numbers per byte and a modulo 210 wheel.
 *
 * Removes the multiples of sieving primes within EratBig from
 * the current segment.
 * @see SieveOfEratosthenes::crossOffMultiples()
 */
void EratBig::sieve(uint8_t* sieve) {
  if (primeCount_ == 0)
    return;
  // get the bucket list related to the current segment,
  // the list contains the sieving primes with multiple occurence(s)
  // in the current segment
  Bucket_t*& bucket = bucketLists_[index_];
  // of type 2^n - 1
  const uint32_t moduloListsSize = size_ - 1;
  const uint32_t moduloSieveSize = (1U << log2SieveSize_) - 1;

  // iterate over the bucket list
  while (bucket != NULL) {
    const uint32_t      count       = bucket->getCount();
    const WheelPrime_t* wheelPrimes = bucket->getWheelPrimes();
    uint32_t i = 0;

    // iterate over the sieving primes within the current bucket
    // loop unrolled by 4
    for (; i < count - count % 4; i += 4) {
      uint32_t sieveIndex0   = wheelPrimes[i+0].getSieveIndex();
      uint32_t wheelIndex0   = wheelPrimes[i+0].getWheelIndex();
      uint32_t sievingPrime0 = wheelPrimes[i+0].getSievingPrime();
      uint32_t sieveIndex1   = wheelPrimes[i+1].getSieveIndex();
      uint32_t wheelIndex1   = wheelPrimes[i+1].getWheelIndex();
      uint32_t sievingPrime1 = wheelPrimes[i+1].getSievingPrime();
      uint32_t sieveIndex2   = wheelPrimes[i+2].getSieveIndex();
      uint32_t wheelIndex2   = wheelPrimes[i+2].getWheelIndex();
      uint32_t sievingPrime2 = wheelPrimes[i+2].getSievingPrime();
      uint32_t sieveIndex3   = wheelPrimes[i+3].getSieveIndex();
      uint32_t wheelIndex3   = wheelPrimes[i+3].getWheelIndex();
      uint32_t sievingPrime3 = wheelPrimes[i+3].getSievingPrime();

      // cross off the multiple (unset corresponding bit) of the
      // current sieving primes within the sieve array
      sieve[sieveIndex0] &= wheel_[wheelIndex0].unsetBit;
      sieveIndex0 += wheel_[wheelIndex0].nextMultipleFactor * sievingPrime0;
      sieveIndex0 += wheel_[wheelIndex0].correct;
      wheelIndex0 += wheel_[wheelIndex0].next;
      sieve[sieveIndex1] &= wheel_[wheelIndex1].unsetBit;
      sieveIndex1 += wheel_[wheelIndex1].nextMultipleFactor * sievingPrime1;
      sieveIndex1 += wheel_[wheelIndex1].correct;
      wheelIndex1 += wheel_[wheelIndex1].next;
      sieve[sieveIndex2] &= wheel_[wheelIndex2].unsetBit;
      sieveIndex2 += wheel_[wheelIndex2].nextMultipleFactor * sievingPrime2;
      sieveIndex2 += wheel_[wheelIndex2].correct;
      wheelIndex2 += wheel_[wheelIndex2].next;
      sieve[sieveIndex3] &= wheel_[wheelIndex3].unsetBit;
      sieveIndex3 += wheel_[wheelIndex3].nextMultipleFactor * sievingPrime3;
      sieveIndex3 += wheel_[wheelIndex3].correct;
      wheelIndex3 += wheel_[wheelIndex3].next;

      uint32_t nextIndex0 = (index_ + (sieveIndex0 >> log2SieveSize_)) & moduloListsSize;
      sieveIndex0 &= moduloSieveSize;
      uint32_t nextIndex1 = (index_ + (sieveIndex1 >> log2SieveSize_)) & moduloListsSize;
      sieveIndex1 &= moduloSieveSize;
      uint32_t nextIndex2 = (index_ + (sieveIndex2 >> log2SieveSize_)) & moduloListsSize;
      sieveIndex2 &= moduloSieveSize;
      uint32_t nextIndex3 = (index_ + (sieveIndex3 >> log2SieveSize_)) & moduloListsSize;
      sieveIndex3 &= moduloSieveSize;

      // move the current sieving primes to the bucket list
      // related to their next multiple occurrence
      if (!bucketLists_[nextIndex0]->addWheelPrime(sievingPrime0, sieveIndex0, wheelIndex0))
        this->pushBucket(nextIndex0);
      if (!bucketLists_[nextIndex1]->addWheelPrime(sievingPrime1, sieveIndex1, wheelIndex1))
        this->pushBucket(nextIndex1);
      if (!bucketLists_[nextIndex2]->addWheelPrime(sievingPrime2, sieveIndex2, wheelIndex2))
        this->pushBucket(nextIndex2);
      if (!bucketLists_[nextIndex3]->addWheelPrime(sievingPrime3, sieveIndex3, wheelIndex3))
        this->pushBucket(nextIndex3);
    }

    // process the remaining sieving primes
    for (; i < count; i++) {
      uint32_t sieveIndex   = wheelPrimes[i].getSieveIndex();
      uint32_t wheelIndex   = wheelPrimes[i].getWheelIndex();
      uint32_t sievingPrime = wheelPrimes[i].getSievingPrime();
      // cross off the multiple (unset corresponding bits) of the
      // current sieving prime within the sieve array
      sieve[sieveIndex] &= wheel_[wheelIndex].unsetBit;
      sieveIndex += wheel_[wheelIndex].nextMultipleFactor * sievingPrime;
      sieveIndex += wheel_[wheelIndex].correct;
      wheelIndex += wheel_[wheelIndex].next;
      uint32_t nextIndex = (index_ + (sieveIndex >> log2SieveSize_)) & moduloListsSize;
      sieveIndex &= moduloSieveSize;
      // move the current sieving prime to the bucket list
      // related to its next multiple occurrence
      if (!bucketLists_[nextIndex]->addWheelPrime(sievingPrime, sieveIndex, wheelIndex))
        this->pushBucket(nextIndex);
    }

    // reset the processed bucket and move it to the bucket stock
    bucket->reset();
    Bucket_t* old = bucket;
    bucket = bucket->next;
    old->next = bucketStock_;
    bucketStock_ = old;
  }

  // no more buckets in the processed bucket list,
  // re-initialize with an empty bucket
  this->pushBucket(index_);
  // increase the list index_ for the next segment
  index_ = (index_ + 1) & moduloListsSize;
}
