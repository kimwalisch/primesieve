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

/** 
 * @file imath.h 
 * @brief Auxiliary integer math functions needed in primesieve.
 */

#ifndef IMATH_PRIMESIEVE_H
#define IMATH_PRIMESIEVE_H

namespace soe {

template <typename T>
inline T isquare(T x) {
  return x * x;
}

/**
 * Integer square root, Newton's method.
 * @see "Hacker's Delight, p. 203-207"
 * @typename T  32 or 64-bit unsigned integer type
 */
template <typename T>
inline T isqrt(T x) {
  T x1;
  T s, g0, g1;

  if (x <= 1) return x;
  s = 1;
  x1 = x - 1;
  if (x1 > 4294967295u) { s += 16; x1 >>= 31; x1 >>= 1; }
  if (x1 > 65535)       { s +=  8; x1 >>= 16; }
  if (x1 > 255)         { s +=  4; x1 >>=  8; }
  if (x1 > 15)          { s +=  2; x1 >>=  4; }
  if (x1 > 3)           { s +=  1; }

  g0 = 1; g0 <<= s;
  g1 = (g0 + (x >> s)) >> 1;

  while (g1 < g0) {
    g0 = g1;
    g1 = (g0 + (x / g0)) >> 1;
  }
  return g0;
}

} // namespace soe

#endif /* IMATH_PRIMESIEVE_H */
