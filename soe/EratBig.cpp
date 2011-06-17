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

#include <cstdlib>
#include <stdexcept>
#include <cassert>

#define BUCKETS_PER_CREATE (defs::MEMORY_PER_ALLOC_ERATBIG / sizeof(Bucket_t))
#define SIEVE_SIZE ((1u << log2SieveSize_) - 1)

EratBig::EratBig(const SieveOfEratosthenes* soe) : Modulo210Wheel(soe),
    log2SieveSize_(floorLog2(soe->getSieveSize())), primeCount_(0), index_(0),
    bucketLists_(NULL), bucketStock_(NULL) {
  // EratBig uses bitwise operations that require a power of 2 sieve size
  if (!isPowerOf2(soe->getSieveSize()))
    throw std::invalid_argument(
        "EratBig: sieveSize must be a power of 2 (2^n).");
  // max sieveSize = max WheelPrime sieveIndex = 2^23
  if (soe->getSieveSize() > (1u << 23))
    throw std::invalid_argument(
        "EratBig: sieveSize must be <= 2^23, 8192 Kilobytes.");
  this->setSize(soe->getSieveSize());
  this->initBucketLists();
}

/**
 * Delete all allocated buckets.
 */
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
void EratBig::setSize(uint32_t sieveSize) {
  assert(sieveSize != 0);
  // greatest possible values of sieveIndex and segmentCount in
  // sieve(uint8_t*)
  double maxSieveIndex = (sieveSize - 1) + ((isqrt(stopNumber_) * 2.0)
      / SieveOfEratosthenes::NUMBERS_PER_BYTE) * wheel_[1].nextMultipleFactor;
  uint32_t maxSegmentCount = static_cast<uint32_t> (std::ceil(
      maxSieveIndex / sieveSize));
  size_ = nextHighestPowerOf2(maxSegmentCount);
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
    this->getBucket(i);
  }
}

/**
 * Add a prime number for sieving to EratBig.
 */
void EratBig::addSievingPrime(uint32_t prime, uint64_t segmentLow) {
  uint32_t sieveIndex;
  uint32_t wheelIndex;
  if (this->setWheelPrime(segmentLow, &prime, &sieveIndex, &wheelIndex)
      == true) {
    // indicates in how many segments the next multiple of prime
    // needs to be eliminated
    uint32_t segmentCount = sieveIndex >> log2SieveSize_;
    // sieve index of prime's next multiple
    sieveIndex &= SIEVE_SIZE;
    // calculate the index of the bucket list associated to the next
    // multiple's segment
    uint32_t nextListIndex = (index_ + segmentCount) & (size_ - 1);
    if (!bucketLists_[nextListIndex]->addWheelPrime(prime, sieveIndex, wheelIndex))
      this->getBucket(nextListIndex);
    primeCount_++;
  }
}

/**
 * Adds an empty bucket from the bucketStock_ to the front of
 * bucketLists_[listIndex], if the bucketStock_ is empty new buckets
 * are allocated first.
 */
void EratBig::getBucket(uint32_t listIndex) {
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
  bucket->next = bucketLists_[listIndex];
  bucketLists_[listIndex] = bucket;
}

/**
 * Implementation of Tomas Oliveira e Silva's cache-friendly segmented
 * sieve of Eratosthenes with wheel factorization (modulo 210 wheel).
 * Is used to cross-off the multiples of the current segment.
 */
void EratBig::sieve(uint8_t* sieve) {
  // nothing to do
  if (primeCount_ == 0)
    return;
  // iterate over the buckets of the current bucket list
  while (bucketLists_[index_] != NULL) {
    // iterate over the wheelPrimes of the current bucket
    WheelPrime* wPrime = bucketLists_[index_]->wheelPrimeBegin();
    WheelPrime* end = bucketLists_[index_]->wheelPrimeEnd();
    while (wPrime != end) {
      // get the current wheelPrime's values
      uint32_t sievingPrime = wPrime->getSievingPrime();
      uint32_t sieveIndex = wPrime->getSieveIndex();
      uint32_t wheelIndex = wPrime->getWheelIndex();
      wPrime++;
      // cross-off the multiples of the current wheelPrime
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
      /// @see addPrimeNumber(uint32_t, uint64_t)
      sieveIndex &= SIEVE_SIZE;
      uint32_t nextListIndex = (index_ + segmentCount) & (size_ - 1);
      if (!bucketLists_[nextListIndex]->addWheelPrime(sievingPrime, sieveIndex, wheelIndex))
        this->getBucket(nextListIndex);
    }
    // reset the finished bucket and move it to the bucket stock
    bucketLists_[index_]->reset();
    Bucket_t* bucket = bucketLists_[index_];
    bucketLists_[index_] = bucketLists_[index_]->next;
    bucket->next = bucketStock_;
    bucketStock_ = bucket;
  }
  // the current bucketList is empty now, add an empty bucket for the
  // next segment
  this->getBucket(index_);
  // increase the bucketLists_ index for the next segment
  index_ = (index_ + 1) & (size_ - 1);
}
