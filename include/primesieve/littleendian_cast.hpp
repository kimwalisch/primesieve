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

#if defined(__BYTE_ORDER__) && \
    defined(__ORDER_BIG_ENDIAN__) && \
    __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__

#include <stdint.h>
#include <cstring>

namespace {

template <typename T>
inline T littleendian_cast(const uint8_t* array)
{
  T result;
  std::memcpy(&result, array, sizeof(T));
  return result;
}

} // namespace

#else

#include <macros.hpp>
#include <stdint.h>

namespace {

template <typename T>
inline T littleendian_cast(const uint8_t* array)
{
  // Disallow unaligned memory accesses
  ASSERT(uintptr_t(array) % sizeof(T) == 0);
  return *reinterpret_cast<const T*>(array);
}

} // namespace

#endif

#endif
