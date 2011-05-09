/*
 * EratBig.h -- This file is part of primesieve
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

#ifndef ERATBIG_H
#define ERATBIG_H

#include "WheelFactorization.h"
#include "settings.h"

#include <stdint.h>
#include <list>

/**
 * Implementation of Tomas Oliveira e Silva's cache friendly sieve
 * of Eratosthenes algorithm. The algorithm is described at:
 * http://www.ieeta.pt/~tos/software/prime_sieve.html
 * My implementation uses 30 numbers per byte and a modulo 210 wheel.
 */
class EratBig: protected Modulo210Wheel {
public:
  EratBig(uint64_t, uint32_t);
  ~EratBig();
  void addPrimeNumber(uint32_t, uint64_t);
  void sieve(uint8_t*);
private:
  typedef Bucket<settings::BUCKETSIZE_ERATBIG> Bucket_t;
  /** Current count of sieving primes within EratBig. */
  uint32_t primeCount_;
  /**
   * Array of singly linked bucket lists. Each list contains the
   * sieving primes of the related segment, @code
   * bucketLists_[index_] @endcode is the list associated to the
   * current segment.
   */
  Bucket_t** bucketLists_;
  /** Singly linked list of empty Buckets (part of Memory pool). */
  Bucket_t* bucketStock_;
  /** Contains the pointers of all allocated Buckets (part of Memory pool). */
  std::list<Bucket_t*> bucketPointers_;
  /** Size of bucketLists_. */
  const uint32_t size_;
  /** Current index of bucketLists_. */
  uint32_t index_;
  /** log2 of the size of the sieve_ array */
  const uint32_t log2SieveSize_;
  uint32_t getSize(uint64_t, uint32_t);
  void initBucketLists();
  void moveFront(Bucket_t*&, Bucket_t*&);
};

#endif /* ERATBIG_H */
