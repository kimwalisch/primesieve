///
/// @file   littleendian_cast.hpp
/// @brief  Cast bytes in ascending address order on both little and
///         big endian CPUs.
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef LITTLEENDIAN_CAST_HPP
#define LITTLEENDIAN_CAST_HPP

#include "config.hpp"

namespace primesieve {

/// http://c-faq.com/misc/endiantest.html
inline bool is_littleendian()
{
  union {
    int word;
    char c[sizeof(int)];
  } x;
  x.word = 1;
  return (x.c[0] == 1);
}

/// Recursively sum bytes using template metaprogramming.
/// e.g. littleendian_cast<int32_t>(array) =
/// return (array[0] <<  0) +
///        (array[1] <<  8) +
///        (array[2] << 16) +
///        (array[3] << 24);
///
template <typename T, int INDEX, int STOP>
struct littleendian_cast_helper
{
  static T sum(const byte_t* array, T n)
  {
    n += static_cast<T>(array[INDEX]) << (INDEX * 8);
    return littleendian_cast_helper<T, INDEX + 1, STOP - 1>::sum(array, n);
  }
};

template <typename T, int INDEX>
struct littleendian_cast_helper<T, INDEX, 0>
{
  static T sum(const byte_t*, T n)
  {
    return n;
  }
};

template <typename T>
inline T littleendian_cast(const byte_t* array)
{
  if (is_littleendian())
    return *reinterpret_cast<const T*>(array);
  return littleendian_cast_helper<T, 0, sizeof(T)>::sum(array, 0);
}

} // namespace primesieve

#endif
