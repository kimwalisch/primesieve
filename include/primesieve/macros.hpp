///
/// @file  macros.hpp
///
/// Copyright (C) 2020 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef MACROS_HPP
#define MACROS_HPP

#ifndef __has_attribute
  #define __has_attribute(x) 0
#endif

#ifndef __has_cpp_attribute
  #define __has_cpp_attribute(x) 0
#endif

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

#if __cplusplus >= 202002L && \
    __has_cpp_attribute(unlikely)
  #define if_unlikely(x) if (x) [[unlikely]]
#elif defined(__GNUC__) || \
      defined(__clang__)
  #define if_unlikely(x) if (__builtin_expect(!!(x), 0))
#else
  #define if_unlikely(x) if (x)
#endif

#if __cplusplus >= 201703L && \
    __has_cpp_attribute(fallthrough)
  #define FALLTHROUGH [[fallthrough]];
#elif __has_attribute(fallthrough)
  #define FALLTHROUGH __attribute__((fallthrough));
#else
  #define FALLTHROUGH // fallthrough
#endif

/// Use [[maybe_unused]] from C++17 once widely supported
#if defined(NDEBUG)
  #define MAYBE_UNUSED(x)
#else
  #define MAYBE_UNUSED(x) x
#endif

#endif
