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

#ifndef ERATSMALL_H
#define ERATSMALL_H

#include "config.h"
#include "WheelFactorization.h"

#include <stdint.h>
#include <list> 

namespace soe {

/// EratSmall is an implementation of the segmented sieve of
/// Eratosthenes optimized for small sieving primes that have many
/// multiples per segment.
///
class EratSmall : public Modulo30Wheel_t {
public:
  EratSmall(uint64_t, uint_t, uint_t);
  uint_t getLimit() const { return limit_; }
  void crossOff(uint8_t*, uint8_t*);
private:
  typedef std::list<Bucket>::iterator BucketIterator_t;
  const uint_t limit_;
  /// List of buckets, holds the sieving primes
  std::list<Bucket> buckets_;
  void store(uint_t, uint_t, uint_t);
  void crossOff(uint8_t*, uint8_t*, Bucket&);
  EratSmall(const EratSmall&);
  EratSmall& operator=(const EratSmall&);
};

} // namespace soe

#endif
