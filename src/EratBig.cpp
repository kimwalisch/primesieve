/*
 * EratBig.cpp -- This file is part of primesieve
 *
 * Copyright (C) 2010 Kim Walisch, <kim.walisch@gmail.com>
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
#include "pmath.h"

#include <stdint.h>
#include <cassert>
#include <cstdlib>
#include <stdexcept>

#define BUCKETS_PER_CREATE (settings::MEMORY_PER_ALLOC_ERATBIG / sizeof(Bucket_t))
#define SIEVE_SIZE ((1u << log2SieveSize_) - 1)

EratBig::EratBig(uint64_t stopNumber, uint32_t sieveSize) :
  Modulo210Wheel(stopNumber, sieveSize), primeCount_(0), bucketLists_(NULL),
      bucketStock_(NULL), size_(getSize(stopNumber, sieveSize)), index_(0),
      log2SieveSize_(floorLog2(sieveSize)) {
  // EratBig uses bitwise operations that require a power of 2 sieve size
  if (!isPowerOf2(sieveSize))
    throw std::invalid_argument(
        "EratBig: sieveSize must be a power of 2 (2^n).");
  // max sieveSize = max WheelPrime sieveIndex = 2^23
  if (sieveSize > (1u << 23))
    throw std::invalid_argument(
        "EratBig: sieveSize must be <= 2^23, 8192 Kilobytes.");
  this->initBucketLists();
}

/**
 * Delete all the buckets that have been allocated.
 */
EratBig::~EratBig() {
  while (!bucketPointers_.empty()) {
    delete[] bucketPointers_.back();
    bucketPointers_.pop_back();
  }
  if (bucketLists_ != NULL)
    delete[] bucketLists_;
}

/**  
 * @return The size of the bucketLists_ array.
 * @remark The size is a power of 2 value which allows use of fast
 *         bitwise operators in sieve(uint8_t*).
 */
uint32_t EratBig::getSize(uint64_t stopNumber, uint32_t sieveSize) {
  assert(sieveSize != 0);
  // greatest possible values of sieveIndex and
  // nextSieveRound in sieve(uint8_t*)
  double maxSieveIndex = (sieveSize - 1) + ((U32SQRT(stopNumber) * 2.0)
      / SieveOfEratosthenes::NUMBERS_PER_BYTE) * wheel_[1].nextMultipleFactor;
  uint32_t maxNextSieveRound = static_cast<uint32_t> (ceil(maxSieveIndex
      / sieveSize));
  return nextHighestPowerOf2(maxNextSieveRound);
}

/**
 * Allocates the bucketLists_ array and initializes each list with an
 * empty bucket.
 */
void EratBig::initBucketLists() {
  assert(size_ > 0);
  bucketLists_ = new Bucket_t*[size_];
  for (uint32_t i = 0; i < size_; i++) {
    bucketLists_[i] = NULL;
    this->moveFrontBucket(bucketLists_[i], bucketStock_);
  }
}

/**
 * Adds a prime number for sieving to EratBig.
 */
void EratBig::addPrimeNumber(uint32_t primeNumber, uint64_t lowerBound) {
  uint32_t sieveIndex;
  uint32_t wheelIndex;
  if (this->setWheelPrime(lowerBound, &primeNumber, &sieveIndex, &wheelIndex)
      == true) {
    // indicates in how many sieve rounds the next multiple of 
    // primeNumber has to be eliminated
    uint32_t nextSieveRound = sieveIndex >> log2SieveSize_;
    // sieve index of primeNumber's next multiple
    sieveIndex &= SIEVE_SIZE;
    // indicates the list that must be used to eliminate the next 
    // multiple of primeNumber
    uint32_t nextListIndex = (index_ + nextSieveRound) & (size_ - 1);
    if (!bucketLists_[nextListIndex]->addWheelPrime(primeNumber, sieveIndex,
        wheelIndex))
      this->moveFrontBucket(bucketLists_[nextListIndex], bucketStock_);
    primeCount_++;
  }
}

/**
 * Moves the first bucket of the source bucketList to the destination
 * bucketList, creates new buckets if the bucketStock_ is empty.
 * @pre src == bucketstock_ || src != NULL
 */
void EratBig::moveFrontBucket(Bucket_t*& dest, Bucket_t*& src) {
  if (bucketStock_ == NULL) {
    Bucket_t* moreBuckets = new Bucket_t[BUCKETS_PER_CREATE];
    for(uint32_t i = 0; i < BUCKETS_PER_CREATE - 1; i++)
      moreBuckets[i].init(&moreBuckets[i + 1]);
    moreBuckets[BUCKETS_PER_CREATE - 1].init(NULL);
    bucketStock_ = &moreBuckets[0];
    bucketPointers_.push_back(moreBuckets);
  }
  assert(src != NULL);
  Bucket_t* bucket = src;
  src = src->next;
  bucket->next = dest;
  dest = bucket;
}

/**
 * Implementation of the segmented sieve of Eratosthenes with wheel
 * factorization (modulo 210 wheel). Is used to cross-off the
 * multiples of the current sieve round.
 */
void EratBig::sieve(uint8_t* sieve) {
  // nothing to do
  if (primeCount_ == 0)
    return;

  // iterate over the buckets of the current bucket list
  while (bucketLists_[index_] != NULL) {
    // iterate over the wheelPrimes of the current bucket
    WheelPrime* wPrime = bucketLists_[index_]->wheelPrimeBegin();
    WheelPrime* end    = bucketLists_[index_]->wheelPrimeEnd();
    while (wPrime != end) {
      // get the current wheelPrime's values
      uint32_t sievePrime = wPrime->getSievePrime();
      uint32_t sieveIndex = wPrime->getSieveIndex();
      uint32_t wheelIndex = wPrime->getWheelIndex();
      wPrime++;
      // eliminate the multiples of the current wheelPrime (of the
      // current sieve round)
      uint32_t nextSieveRound;
      do {
        uint8_t bit = wheel_[wheelIndex].unsetBit;
        uint8_t nmf = wheel_[wheelIndex].nextMultipleFactor;
        uint8_t cor = wheel_[wheelIndex].correct;
         int8_t nxt = wheel_[wheelIndex].next;
        sieve[sieveIndex] &= bit;
        wheelIndex += nxt;
        sieveIndex += sievePrime * nmf + cor;
        nextSieveRound = sieveIndex >> log2SieveSize_;
      } while (nextSieveRound == 0);

      /// @see addPrimeNumber(uint32_t, uint64_t)
      sieveIndex &= SIEVE_SIZE;
      uint32_t nextListIndex = (index_ + nextSieveRound) & (size_ - 1);
      if (!bucketLists_[nextListIndex]->addWheelPrime(sievePrime, sieveIndex,
          wheelIndex))
        this->moveFrontBucket(bucketLists_[nextListIndex], bucketStock_);
    }
    bucketLists_[index_]->reset();
    // move the current empty bucket to the bucket stock
    this->moveFrontBucket(bucketStock_, bucketLists_[index_]);
  }
  // the current bucketList is empty now, add an empty
  // bucket for the next sieve round
  this->moveFrontBucket(bucketLists_[index_], bucketStock_);
  // increase the bucketLists_ index for the next sieve round
  index_ = (index_ + 1) & (size_ - 1);
}
