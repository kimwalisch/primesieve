///
/// @file   endiansafe_cast.h
/// @brief  Cast bytes in ascending address order.
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef ENDIANSAFE_CAST_H
#define ENDIANSAFE_CAST_H

#include "config.h"

namespace soe {

/// http://c-faq.com/misc/endiantest.html
inline bool isLittleEndian()
{
  union {
    int word;
    char c[sizeof(int)];
  } x;
  x.word = 1;
  return (x.c[0] == 1);
}

/// Recursively sum bytes using template metaprogramming.
/// e.g. endiansafe_cast<int32_t>(array) =
/// return (array[0] << 0) +
///        (array[1] << 8) +
///        (array[2] << 16) +
///        (array[3] << 24) +
///        0;

template <typename T, int INDEX, int STOP>
struct endiansafe_cast_helper
{
  static T sum(const byte_t* array)
  {
    T byte = array[INDEX];
    T rest = endiansafe_cast_helper<T, INDEX + 1, STOP - 1>::sum(array);
    return (byte << (INDEX * 8)) + rest;
  }
};

template <typename T, int INDEX>
struct endiansafe_cast_helper<T, INDEX, 0>
{
  static T sum(const byte_t*)
  {
    return 0;
  }
};

template <typename T>
inline T endiansafe_cast(const byte_t* array)
{
  if (isLittleEndian())
    return *reinterpret_cast<const T*>(array);
  return endiansafe_cast_helper<T, 0, sizeof(T)>::sum(array);
}

} // namespace soe

#endif
