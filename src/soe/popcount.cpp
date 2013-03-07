///
/// @file   popcount.cpp
/// @brief  Fast algorithm to count the number of 1 bits in an array.
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#if !defined(__STDC_CONSTANT_MACROS)
  #define __STDC_CONSTANT_MACROS
#endif

#include <stdint.h>

namespace soe {

/// This algorithm counts the number of 1 bits (population count) in
/// an array using 64-bit tree merging. To the best of my knowledge
/// this is the fastest integer arithmetic bit population count
/// algorithm, it uses only 8 operations for 8 bytes on 64-bit CPUs.
/// The 64-bit tree merging popcount algorithm is due to
/// Cédric Lauradoux, it is described in his paper:
/// http://perso.citi.insa-lyon.fr/claurado/ham/overview.pdf
/// http://perso.citi.insa-lyon.fr/claurado/hamming.html
///
uint64_t popcount(const uint64_t* array, uint64_t size)
{
  const uint64_t m1  = UINT64_C(0x5555555555555555);
  const uint64_t m2  = UINT64_C(0x3333333333333333);
  const uint64_t m4  = UINT64_C(0x0F0F0F0F0F0F0F0F);
  const uint64_t m8  = UINT64_C(0x00FF00FF00FF00FF);
  const uint64_t h01 = UINT64_C(0x0101010101010101);

  uint64_t limit30 = size - size % 30;
  uint64_t i, j;
  uint64_t count1, count2, half1, half2, acc;
  uint64_t bit_count = 0;
  uint64_t x;

  if (array == 0)
    return 0;

  // 64-bit tree merging (merging3)
  for (i = 0; i < limit30; i += 30, array += 30) {
    acc = 0;
    for (j = 0; j < 30; j += 3) {
      count1  =  array[j];
      count2  =  array[j+1];
      half1   =  array[j+2];
      half2   =  array[j+2];
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
    acc = (acc & m8) + ((acc >>  8) & m8);
    acc = (acc       +  (acc >> 16));
    acc = (acc       +  (acc >> 32));
    bit_count += acc & 0xffff;
  }

  // Count the bits of the remaining bytes (max 29*8 = 232)
  // http://en.wikipedia.org/wiki/Hamming_weight#Efficient_implementation
  for (i = 0; i < size - limit30; i++) {
    x = array[i];
    x =  x       - ((x >> 1)  & m1);
    x = (x & m2) + ((x >> 2)  & m2);
    x = (x       +  (x >> 4)) & m4;
    x = (x * h01) >> 56;
    bit_count += x;
  }

  return bit_count;
}

} // namespace soe
