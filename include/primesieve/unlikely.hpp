///
/// @file  unlikely.hpp
///
/// Copyright (C) 2020 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef UNLIKELY_HPP
#define UNLIKELY_HPP

#if __cplusplus >= 201803L
  #define if_unlikely(x) if (x) [[unlikely]]
#elif defined(__GNUC__) || \
      defined(__clang__)
  #define if_unlikely(x) if (__builtin_expect(!!(x), 0))
#else
  #define if_unlikely(x) if (x)
#endif

#endif
