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


/// @file imath.h
/// @brief Auxiliary integer math functions needed in primesieve.

#ifndef IMATH_PRIMESIEVE_H
#define IMATH_PRIMESIEVE_H

namespace soe {

template <typename T>
inline T isquare(T x)
{
  return x * x;
}

template <typename T>
inline T getNumberOfBits()
{
  return static_cast<T>(sizeof(T) * 8);
}

/// Determine if an integer is a power of 2.
/// @param x  Integer value.
///
template <typename T>
inline bool isPowerOf2(T x)
{
  return (x != 0 && (x & (x - 1)) == 0);
}

/// Round down to the next power of 2.
/// @see Hacker's Delight, p. 47.
/// @param x  Integer value.
///
template <typename T>
inline T floorPowerOf2(T x)
{
  for (T i = 1; i < getNumberOfBits<T>(); i += i)
    x |= (x >> i);
  return x - (x >> 1);
}

/// Fast and protable integer log2 function.
/// @see Hacker's Delight, p. 215.
/// @param x  Integer value.
///
template <typename T>
inline T ilog2(T x)
{
  const T bits = getNumberOfBits<T>();
  const T one = 1;
  T log2 = 0;
  for (T i = bits / 2; i != 0; i /= 2)
    if (x >= one << i) { x >>= i; log2 += i; }
  return log2;
}

/// Integer square root, Newton's method.
/// @see Hacker's Delight, p. 203-207.
/// @param x  Unsigned integer.
///
template <typename T>
inline T isqrt(T x)
{
  if (x <= 1) return x;
  const T bits = getNumberOfBits<T>();
  const T one = 1;

  // s      = bits / 2 - nlz(x - 1) / 2
  // nlz(x) = bits - 1 - ilog2(x)
  T s = bits / 2 - (bits - 1) / 2 + ilog2(x - 1) / 2;

  // first guess: least power of 2 >= sqrt(x)
  T g0 = one << s;
  T g1 = (g0 + (x >> s)) >> 1;

  while (g1 < g0) {
    g0 = g1;
    g1 = (g0 + (x / g0)) >> 1;
  }
  return g0;
}

template <typename T>
inline T getInBetween(T min, T value, T max)
{
  if (value < min) return min;
  if (value > max) return max;
  return value;
}

} // namespace soe

#endif
