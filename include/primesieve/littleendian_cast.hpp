///
/// @file   littleendian_cast.hpp
/// @brief  Cast bytes in ascending address order on both little and
///         big endian CPUs.
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef LITTLEENDIAN_CAST_HPP
#define LITTLEENDIAN_CAST_HPP

#include "macros.hpp"
#include <stdint.h>

namespace {

template <typename T>
inline T littleendian_cast(const uint8_t* array)
{
  // Disallow unaligned memory accesses
  ASSERT(uintptr_t(array) % sizeof(T) == 0);

#if defined(__BYTE_ORDER__) && \
    defined(__ORDER_BIG_ENDIAN__) && \
    __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__

  static_assert(sizeof(T) == sizeof(uint64_t), "Type T must be uint64_t!");
  return __builtin_bswap64(*reinterpret_cast<const T*>(array));
#else
  return *reinterpret_cast<const T*>(array);
#endif
}

} // namespace

#endif
