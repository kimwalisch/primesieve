/*
 * EratBig.cpp -- This file is part of primesieve
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

#include "EratBig.h"
#include "SieveOfEratosthenes.h"
#include "WheelFactorization.h"
#include "defs.h"
#include "pmath.h"

#include <stdexcept>
#include <cstdlib>
#include <cassert>

#define BUCKETS_PER_CREATE (defs::ERATBIG_MEMORY_PER_ALLOC / sizeof(Bucket_t))

EratBig::EratBig(const SieveOfEratosthenes& soe) : Modulo210Wheel(soe),
    log2SieveSize_(floorLog2(soe.getSieveSize())),
    moduloSieveSize_(soe.getSieveSize() - 1),
    primeCount_(0), index_(0), bucketLists_(NULL), bucketStock_(NULL) {
  // EratBig uses bitwise operations that require a power of 2 sieve size
  if (!isPowerOf2(soe.getSieveSize()))
    throw std::invalid_argument(
        "EratBig: sieveSize must be a power of 2 (2^n).");
  this->setSize(soe);
  this->initBucketLists();
}

EratBig::~EratBig() {
  while (!bucketPointers_.empty()) {
    delete[] bucketPointers_.back();
    bucketPointers_.pop_back();
  }
  delete[] bucketLists_;
}

/**  
 * Set the size of the bucketLists_ array.
 * @remark The size is a power of 2 value which allows use of fast
 *         bitwise operators in sieve(uint8_t*).
 */
void EratBig::setSize(const SieveOfEratosthenes& soe) {
  uint32_t sieveSize = soe.getSieveSize();
  uint32_t sqrtStop = isqrt(soe.getStopNumber());
  // MAX values in sieve(uint8_t*)
  uint32_t maxSievingPrime = sqrtStop / (SieveOfEratosthenes::NUMBERS_PER_BYTE / 2);
  uint32_t maxWheelFactor = wheel_[1].nextMultipleFactor;
  uint32_t maxSieveIndex = (sieveSize - 1) + maxSievingPrime * maxWheelFactor;
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
 */
void EratBig::addSievingPrime(uint32_t prime) {
  uint32_t sieveIndex;
  uint32_t wheelIndex;
  if (this->getWheelPrimeData(&prime, &sieveIndex, &wheelIndex)
      == true) {
    // indicates in how many segments the next multiple of prime
    // needs to be crossed off
    uint32_t segmentCount = sieveIndex >> log2SieveSize_;
    // position of the next multiple of prime within the sieve array
    sieveIndex &= moduloSieveSize_;
    // calculate the list that will be used to remove the next
    // multiple of prime
    uint32_t nextIndex = (index_ + segmentCount) & (size_ - 1);
    if (!bucketLists_[nextIndex]->addWheelPrime(prime, sieveIndex, wheelIndex))
      this->pushBucket(nextIndex);
    primeCount_++;
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
    for(uint32_t i = 0; i < BUCKETS_PER_CREATE - 1; i++)
      more[i].setNext(&more[i+1]);
    more[BUCKETS_PER_CREATE-1].setNext(NULL);
    bucketStock_ = &more[0];
    assert(bucketStock_ != NULL);
    bucketPointers_.push_back(more);
  }
  Bucket_t* bucket = bucketStock_;
  bucketStock_ = bucketStock_->next;
  bucket->next = bucketLists_[index];
  bucketLists_[index] = bucket;
}

/**
 * Implementation of Tomas Oliveira e Silva's cache-friendly segmented
 * sieve of Eratosthenes. The algorithm is described at:
 * http://www.ieeta.pt/~tos/software/prime_sieve.html
 * My implementation uses 30 numbers per byte and a modulo 210 wheel.
 *
 * Removes the multiples (of sieving primes within EratBig) from the
 * current segment.
 */
void EratBig::sieve(uint8_t* sieve) {
  if (primeCount_ == 0)
    return;
  // iterate over the sieving primes with multiple occurrences in the
  // current segment
  while (bucketLists_[index_] != NULL) {
    WheelPrime* wPrime = bucketLists_[index_]->begin();
    WheelPrime* end = bucketLists_[index_]->end();
    while (wPrime != end) {
      uint32_t sievingPrime = wPrime->getSievingPrime();
      uint32_t sieveIndex = wPrime->getSieveIndex();
      uint32_t wheelIndex = wPrime->getWheelIndex();
      wPrime++;
      // remove the multiples of the current sievingPrime from the
      // sieve array (i.e. the current segment)
      uint32_t segmentCount;
      do {
        uint8_t bit = wheel_[wheelIndex].unsetBit;
        uint8_t nmf = wheel_[wheelIndex].nextMultipleFactor;
        uint8_t cor = wheel_[wheelIndex].correct;
         int8_t nxt = wheel_[wheelIndex].next;
        sieve[sieveIndex] &= bit;
        wheelIndex += nxt;
        sieveIndex += sievingPrime * nmf + cor;
        segmentCount = sieveIndex >> log2SieveSize_;
      } while (segmentCount == 0);
      /// @see addSievingPrime(uint32_t, uint64_t)
      sieveIndex &= moduloSieveSize_;
      uint32_t nextIndex = (index_ + segmentCount) & (size_ - 1);
      if (!bucketLists_[nextIndex]->addWheelPrime(sievingPrime, sieveIndex, wheelIndex))
        this->pushBucket(nextIndex);
    }
    // reset the processed bucket and move it to the bucket stock
    bucketLists_[index_]->reset();
    Bucket_t* bucket = bucketLists_[index_];
    bucketLists_[index_] = bucketLists_[index_]->next;
    bucket->next = bucketStock_;
    bucketStock_ = bucket;
  }
  // no more buckets in the current bucketList, add an empty bucket
  // for the next segment
  this->pushBucket(index_);
  // increase index_ for the next segment
  index_ = (index_ + 1) & (size_ - 1);
}
