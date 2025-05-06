///
/// @file  macros.hpp
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef MACROS_HPP
#define MACROS_HPP

#ifndef __has_attribute
  #define __has_attribute(x) 0
#endif

#ifndef __has_builtin
  #define __has_builtin(x) 0
#endif

#ifndef __has_cpp_attribute
  #define __has_cpp_attribute(x) 0
#endif

#ifndef __has_include
  #define __has_include(x) 0
#endif

// Required for std::unreachable()
#include <utility>

/// Enable expensive debugging assertions.
/// These assertions enable e.g. bounds checks for the
/// Vector and Array types.
///
#if defined(ENABLE_ASSERT)
  #undef NDEBUG
  #include <cassert>
  #define ASSERT(x) assert(x)
#else
  #define ASSERT(x) (static_cast<void>(0))
#endif

#if __has_attribute(always_inline)
  #define ALWAYS_INLINE inline __attribute__((always_inline))
#elif defined(_MSC_VER)
  #define ALWAYS_INLINE inline __forceinline
#else
  #define ALWAYS_INLINE inline
#endif

/// Some functions in primesieve use a large number of variables
/// at the same time. If such functions are inlined then
/// performance drops because not all variables fit into registers
/// which causes register spilling. We annotate such functions
/// with NOINLINE in order to avoid these issues.
///
#if __has_attribute(noinline)
  #define NOINLINE __attribute__((noinline))
#elif defined(_MSC_VER)
  #define NOINLINE __declspec(noinline)
#else
  #define NOINLINE
#endif

#if __cplusplus >= 202002L && \
    __has_cpp_attribute(likely)
  #define if_likely(x) if (x) [[likely]]
#elif defined(__GNUC__) || \
      __has_builtin(__builtin_expect)
  #define if_likely(x) if (__builtin_expect(!!(x), 1))
#else
  #define if_likely(x) if (x)
#endif

#if __cplusplus >= 202002L && \
    __has_cpp_attribute(unlikely)
  #define if_unlikely(x) if (x) [[unlikely]]
#elif defined(__GNUC__) || \
      __has_builtin(__builtin_expect)
  #define if_unlikely(x) if (__builtin_expect(!!(x), 0))
#else
  #define if_unlikely(x) if (x)
#endif

#if __cplusplus >= 201703L && \
    __has_cpp_attribute(fallthrough)
  #define FALLTHROUGH [[fallthrough]]
#elif __has_attribute(fallthrough)
  #define FALLTHROUGH __attribute__((fallthrough))
#else
  #define FALLTHROUGH
#endif

#if defined(__GNUC__) || \
    __has_builtin(__builtin_unreachable)
  #define UNREACHABLE __builtin_unreachable()
#elif defined(_MSC_VER)
  #define UNREACHABLE __assume(0)
#elif __cplusplus >= 202301L && \
      defined(__cpp_lib_unreachable)
  // We prefer __builtin_unreachable() over std::unreachable()
  // because GCC's std::unreachable() implementation uses
  // __builtin_trap() instead of __builtin_unreachable() if
  // _GLIBCXX_ASSERTIONS is defined.
  #define UNREACHABLE std::unreachable()
#else
  #define UNREACHABLE
#endif

#endif
