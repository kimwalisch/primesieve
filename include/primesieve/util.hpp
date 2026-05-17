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

/// Safely loads a trivially copyable type T from a byte
/// array. Employs std::memcpy to bypass strict aliasing
/// and avoid UB. Uses C++20 std::assume_aligned to
/// guarantee optimal assembly code gen.
///
template <typename T>
ALWAYS_INLINE T load_aligned(const uint8_t* array)
{
  static_assert(std::is_trivially_copyable<T>::value, "Type T must be trivially copyable!");
  ASSERT(uintptr_t(array) % alignof(T) == 0);

#if defined(__cpp_lib_assume_aligned)
  array = std::assume_aligned<alignof(T)>(array);
#endif

  T result;
  std::memcpy(&result, array, sizeof(T));
  return result;
}

template <typename T>
ALWAYS_INLINE T littleendian_cast(const uint8_t* array)
{
#if defined(__BYTE_ORDER__) && \
    defined(__ORDER_BIG_ENDIAN__) && \
    __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__

  static_assert(sizeof(T) == sizeof(uint64_t), "Type T must be uint64_t!");
  return __builtin_bswap64(load_aligned<T>(array));
#else
  return load_aligned<T>(array);
#endif
}

} // namespace

#endif
