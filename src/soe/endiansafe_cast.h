///
/// @file   endiansafe_cast.h
/// @brief  Cast bytes in ascending address order.
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the New BSD License. See the
/// LICENSE file in the top level directory.
///

#ifndef ENDIANSAFE_CAST_H
#define ENDIANSAFE_CAST_H

#include "config.h"
#include <cstddef>

namespace soe {

/// Recursively sum bytes using template metaprogramming.
/// e.g. endiansafe_cast<int32_t>(array) = 
/// return (array[0] <<  0) +
///        (array[1] <<  8) +
///        (array[2] << 16) +
///        (array[3] << 24) +
///        0;
///
template <typename T, std::size_t COUNT>
struct endiansafe_cast_helper
{
  enum {
    INDEX = sizeof(T) - COUNT
  };
  static T go(const byte_t* array)
  {
    T byte = array[INDEX];
    return (byte << INDEX * 8) + endiansafe_cast_helper<T, COUNT - 1>::go(array);
  }
};

template <typename T>
struct endiansafe_cast_helper<T, 0>
{
  static T go(const byte_t*)
  {
    return 0;
  }
};

template <typename T>
inline T endiansafe_cast(const byte_t* array)
{
  return endiansafe_cast_helper<T, sizeof(T)>::go(array);
}

} // namespace soe

#endif
