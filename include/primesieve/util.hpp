///
/// @file   util.hpp
/// @brief  Utility functions.
///
/// Copyright (C) 2026 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef UTIL_HPP
#define UTIL_HPP

#include "macros.hpp"

#include <stdint.h>
#include <cstring>
#include <memory>
#include <type_traits>

namespace {

/// Load a trivially copyable type from aligned byte storage.
/// std::memcpy avoids undefined behavior due to type aliasing
/// and object lifetime rules. Compilers optimize this to a
/// plain load when the size is known at compile time.
template <typename T>
ALWAYS_INLINE T load_aligned(const uint8_t* array)
{
  static_assert(std::is_trivially_copyable<T>::value, "Type T must be trivially copyable!");
  ASSERT(((uintptr_t) array) % alignof(T) == 0);

#if defined(__GNUC__) || \
    defined(__clang__)
  array = (const uint8_t*) __builtin_assume_aligned(array, alignof(T));
#elif defined(__cpp_lib_assume_aligned)
  array = std::assume_aligned<alignof(T)>(array);
#endif

  T result;
  std::memcpy(&result, array, sizeof(T));
  return result;
}

/// Cast bytes in ascending address order on both little and
/// big endian CPUs.
template <typename T>
ALWAYS_INLINE T littleendian_cast(const uint8_t* array)
{
#if defined(__BYTE_ORDER__) && \
    defined(__ORDER_BIG_ENDIAN__) && \
    __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__

  static_assert(sizeof(T) == sizeof(uint64_t), "Type T must be uint64_t!");
  return (T) __builtin_bswap64(load_aligned<T>(array));
#else
  return load_aligned<T>(array);
#endif
}

} // namespace

#endif
