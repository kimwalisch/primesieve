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

#ifndef ERATBIG_H
#define ERATBIG_H

#include "config.h"
#include "WheelFactorization.h"

#include <stdint.h>
#include <vector>
#include <list>

namespace soe {
class SieveOfEratosthenes;

/// EratBig is an implementation of the segmented sieve of
/// Eratosthenes optimized for big sieving primes that have very few
/// multiples per segment.
///
class EratBig: public Modulo210Wheel_t {
public:
  EratBig(const SieveOfEratosthenes&);
  ~EratBig();
  void crossOff(uint8_t*);
private:
  typedef std::list<Bucket*>::iterator PointerIterator_t;
  enum { BUCKETS_PER_ALLOC = config::MEMORY_PER_ALLOC / sizeof(Bucket) };
  /// Vector of bucket lists, holds the sieving primes
  std::vector<Bucket*> lists_;
  /// List of empty buckets
  Bucket* stock_;
  /// Pointers of the allocated buckets
  std::list<Bucket*> pointers_;
  /// log2 of SieveOfEratosthenes::sieveSize_
  const uint_t log2SieveSize_;
  const uint_t moduloSieveSize_;
  void setListsSize(const SieveOfEratosthenes&);
  void init();
  static void moveBucket(Bucket&, Bucket*&);
  void pushBucket(Bucket*&);
  void storeSievingPrime(uint_t, uint_t, uint_t);
  void crossOff(Bucket&, uint8_t*);
  Bucket*& getList(uint_t*);
};

} // namespace soe

#endif
