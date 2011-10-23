//
// Copyright (c) 2011 Kim Walisch, <kim.walisch@gmail.com>.
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

/** 
 * @file bithacks.h 
 * @brief Contains bit manipulation functions needed in primesieve.
 */

#ifndef BITHACKS_H
#define BITHACKS_H

#include "defs.h"

#include <cstdlib>
#include <cassert>

/**
 * Count the number of 1 bits (population count) in an array using
 * 64-bit tree merging. This implementation uses only 8 operations per
 * 8 bytes on 64-bit CPUs, according to my benchmarks it is about as
 * fast as the SSE4.2 popcnt_u32 instruction.
 *
 * The algorithm is due to Cédric Lauradoux, it is described and
 * benchmarked against other bit population count solutions (lookup
 * tables, bit-slicing) in his paper:
 * http://perso.citi.insa-lyon.fr/claurado/ham/overview.pdf
 * http://perso.citi.insa-lyon.fr/claurado/hamming.html
 *
 * The paper and the corresponding source archive (C code) are also
 * available from:
 * http://primesieve.googlecode.com/svn/claurado/hamming-weight/overview.pdf
 * http://primesieve.googlecode.com/svn/claurado/hamming-weight/all.tar.gz
 */
inline uint32_t popcount_lauradoux(const uint64_t* data, uint32_t size) {
  assert(data != NULL);
  assert(size <= UINT32_MAX / (8 * sizeof(uint64_t)));

  const uint64_t m1  = UINT64_C(0x5555555555555555);
  const uint64_t m2  = UINT64_C(0x3333333333333333);
  const uint64_t m4  = UINT64_C(0x0F0F0F0F0F0F0F0F);
  const uint64_t m8  = UINT64_C(0x00FF00FF00FF00FF);
  const uint64_t m16 = UINT64_C(0x0000FFFF0000FFFF);
  const uint64_t h01 = UINT64_C(0x0101010101010101);

  uint32_t bitCount = 0;
  uint32_t i, j;
  uint64_t count1, count2, half1, half2, acc;
  uint64_t x;
  uint32_t limit30 = size - size % 30;

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
    bitCount += (uint32_t)acc;
  }

  // count the bits of the remaining bytes (MAX 29*8) using 
  // "Counting bits set, in parallel" from the "Bit Twiddling Hacks",
  // the code uses wikipedia's 64-bit popcount_3() implementation:
  // http://en.wikipedia.org/wiki/Hamming_weight#Efficient_implementation
  for (i = 0; i < size - limit30; i++) {
    x = data[i];
    x =  x       - ((x >> 1)  & m1);
    x = (x & m2) + ((x >> 2)  & m2);
    x = (x       +  (x >> 4)) & m4;
    bitCount += (uint32_t)((x * h01) >> 56);
  }
  return bitCount;
}

/**
 * Count the number of 1 bits (population count) in a small array
 * using "Counting bits set, Brian Kernighan's way" from the
 * "Bit Twiddling Hacks":
 * http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetKernighan
 */
inline uint32_t popcount_kernighan(const uint8_t* data, uint32_t size) {
  assert(data != NULL);
  assert(size <= UINT32_MAX / (8 * sizeof(uint8_t)));
  uint32_t bitCount = 0;

  for (uint32_t i = 0; i < size; i++) {
    for (uint32_t v = data[i]; v != 0; v &= v - 1)
      bitCount++;
  }
  return bitCount;
}

/**
 * Search the operand (v) for the least significant set bit (1 bit)
 * and return its position.
 * Code from the "Bit Twiddling Hacks":
 * http://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightMultLookup
 * @pre v != 0
 */
inline uint32_t bitScanForward(uint32_t v) {
  assert(v != 0);
  static const uint32_t MultiplyDeBruijnBitPosition[32] = {
       0,  1, 28,  2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17,  4, 8,
      31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18,  6, 11,  5, 10, 9
  };
  const uint32_t i = (uint32_t)-(int32_t)v;
  return MultiplyDeBruijnBitPosition[((v & i) * 0x077CB531U) >> 27];
}

/**
 * Fast and protable integer log2 function.
 * Code from Juan Pablo:
 * http://www.southwindsgames.com/blog/2009/01/19/fast-integer-log2-function-in-cc/
 */
inline uint32_t floorLog2(uint32_t x) {
  uint32_t log2 = 0;
  if (x >= (1 << 16)) { x >>= 16; log2 |= 16; }
  if (x >= (1 <<  8)) { x >>=  8; log2 |=  8; }
  if (x >= (1 <<  4)) { x >>=  4; log2 |=  4; }
  if (x >= (1 <<  2)) { x >>=  2; log2 |=  2; }
  if (x >= (1 <<  1)) {           log2 |=  1; }
  return log2;
}

/**
 * Round up to the next highest power of 2.
 * Code from: "Hacker's Delight, p. 48"
 */
inline uint32_t nextHighestPowerOf2(uint32_t x) {
  x = x - 1;
  x = x | (x >> 1);
  x = x | (x >> 2);
  x = x | (x >> 4);
  x = x | (x >> 8);
  x = x | (x >> 16);
  return x + 1;
}

/**
 * Determine if an integer is a power of 2.
 * Code from the "Bit Twiddling Hacks":
 * http://graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2
 */
inline bool isPowerOf2(uint32_t x) {
  return (x != 0 && (x & (x - 1)) == 0);
}

#endif /* BITHACKS_H */
