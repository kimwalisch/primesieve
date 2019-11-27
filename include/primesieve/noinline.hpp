///
/// @file  noinline.hpp
///
/// Copyright (C) 2019 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef NOINLINE_HPP
#define NOINLINE_HPP

/// Some functions in primesieve use a large number of variables
/// at the same time. If such functions are inlined then
/// performance drops because not all variables fit into registers
/// which causes register spilling. We annotate such functions
/// with NOINLINE in order to avoid these issues.
///
#if defined(_MSC_VER)
  #define NOINLINE __declspec(noinline)
#elif defined(__GNUC__) || \
      defined(__clang__)
  #define NOINLINE __attribute__((noinline))
#else
  #define NOINLINE
#endif

#endif
