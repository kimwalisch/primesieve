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

namespace {

ALWAYS_INLINE uint64_t to_littleendian(uint64_t x)
{
#if defined(__BYTE_ORDER__) && \
    defined(__ORDER_BIG_ENDIAN__) && \
    __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  return __builtin_bswap64(x);
#else
  return x;
#endif
}

} // namespace

#endif
