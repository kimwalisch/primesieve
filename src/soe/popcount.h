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

#ifndef POPCOUNT_PRIMESIEVE_H
#define POPCOUNT_PRIMESIEVE_H

#include "config.h"

#include <stdint.h>
#include <cstdlib>
#include <cassert>

namespace soe {

/// Count the number of 1 bits (population count) in an array using
/// 64-bit tree merging. This implementation uses only 8 operations per
/// 8 bytes on 64-bit CPUs, according to my benchmarks it is about as
/// fast as the SSE4.2 popcnt_u32 instruction.
///
/// The algorithm is due to Cédric Lauradoux, it is described and
/// benchmarked against other bit population count solutions (lookup
/// tables, bit-slicing) in his paper:
/// http://perso.citi.insa-lyon.fr/claurado/ham/overview.pdf
/// http://perso.citi.insa-lyon.fr/claurado/hamming.html
///
/// The paper and the corresponding source archive (C code) are also
/// available from:
/// http://primesieve.googlecode.com/svn/claurado/hamming-weight/overview.pdf
/// http://primesieve.googlecode.com/svn/claurado/hamming-weight/all.tar.gz
///
template <typename T>
inline T popcount_lauradoux(const uint64_t* data, T size) {
  assert(data != NULL);
  const uint64_t m1  = UINT64_C(0x5555555555555555);
  const uint64_t m2  = UINT64_C(0x3333333333333333);
  const uint64_t m4  = UINT64_C(0x0F0F0F0F0F0F0F0F);
  const uint64_t m8  = UINT64_C(0x00FF00FF00FF00FF);
  const uint64_t m16 = UINT64_C(0x0000FFFF0000FFFF);
  const uint64_t h01 = UINT64_C(0x0101010101010101);

  uint64_t count1, count2, half1, half2, acc;
  uint64_t x;
  T bitCount = 0;
  T i, j;
  T limit30 = size - size % 30;

  // 64-bit tree merging (merging3)
  for (i = 0; i < limit30; i += 30, data += 30) {
    acc = 0;
    for (j = 0; j < 30; j += 3) {
      count1  =  data[j];
      count2  =  data[j+1];
      half1   =  data[j+2];
      half2   =  data[j+2];
      half1  &=  m1;
      half2   = (half2  >> 1) & m1;
      count1 -= (count1 >> 1) & m1;
      count2 -= (count2 >> 1) & m1;
      count1 +=  half1;
      count2 +=  half2;
      count1  = (count1 & m2) + ((count1 >> 2) & m2);
      count1 += (count2 & m2) + ((count2 >> 2) & m2);
      acc    += (count1 & m4) + ((count1 >> 4) & m4);
    }
    acc = (acc & m8) + ((acc >>  8)  & m8);
    acc = (acc       +  (acc >> 16)) & m16;
    acc =  acc       +  (acc >> 32);

    bitCount += static_cast<T>(acc);
  }

  // Count the bits of the remaining bytes (max 29*8 = 232) using 
  // "Counting bits set, in parallel" from the "Bit Twiddling Hacks",
  // the code uses wikipedia's 64-bit popcount_3() implementation:
  // http://en.wikipedia.org/wiki/Hamming_weight#Efficient_implementation
  for (i = 0; i < size - limit30; i++) {
    x = data[i];
    x =  x       - ((x >> 1)  & m1);
    x = (x & m2) + ((x >> 2)  & m2);
    x = (x       +  (x >> 4)) & m4;
    bitCount += static_cast<T>((x * h01) >> 56);
  }
  return bitCount;
}

/// Count the number of 1 bits (population count) in a small array
/// using Brian Kernighan's method:
/// http://graphics.stanford.edu/~seander/popcount.html#CountBitsSetKernighan
///
template <typename T>
inline T popcount_kernighan(const uint8_t* data, T size) {
  assert(data != NULL);
  T bitCount = 0;
  for (T i = 0; i < size; i++) {
    for (unsigned int v = data[i]; v != 0; v &= v - 1)
      bitCount++;
  }
  return bitCount;
}

} // namespace soe

#endif
