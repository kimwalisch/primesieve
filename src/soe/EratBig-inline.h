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

#ifndef ERATBIG_INLINE_H
#define ERATBIG_INLINE_H

#include "config.h"
#include "WheelFactorization.h"
#include "EratBig.h"

#include <stdint.h>
#include <vector>

namespace soe {

/// Add a prime number <= sqrt(n) for sieving to EratBig.
/// @see sieve() in SieveOfEratosthenes-inline.h
///
inline void EratBig::addSievingPrime(uint64_t segmentLow, uint32_t prime) {
  uint32_t multipleIndex;
  uint32_t wheelIndex;
  bool store = getWheelPrimeData(segmentLow, &prime, &multipleIndex, &wheelIndex);
  if (store == true) {
    // indicates in how many segments the next multiple
    // of prime needs to be crossed-off
    uint32_t segmentCount = multipleIndex >> log2SieveSize_;
    multipleIndex &= moduloSieveSize_;
    uint32_t next = segmentCount & moduloListsSize_;
    // add prime to the bucket list related
    // to its next multiple occurrence
    if (!lists_[next]->addWheelPrime(prime, multipleIndex, wheelIndex))
      pushBucket(next);
  }
}

} // namespace soe

#endif
