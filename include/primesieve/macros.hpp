///
/// @file  macros.hpp
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
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

#if !defined(__has_include)
  #define __has_include(x) 0
#endif

#if __cplusplus >= 202002L && \
    __has_include(<version>)
  // Required for __cpp_lib_unreachable
  #include <version>
#endif

/// Enable expensive debugging assertions.
/// These assertions enable e.g. bounds checks for the
/// pod_vector and pod_array types.
///
#if defined(ENABLE_ASSERT)
  #undef NDEBUG
  #include <cassert>
  #define ASSERT(x) assert(x)
#else
  #define ASSERT(x) (static_cast<void>(0))
#endif

#if __has_attribute(always_inline)
  #define ALWAYS_INLINE __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
  #define ALWAYS_INLINE __forceinline
#else
  #define ALWAYS_INLINE
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

#if __cplusplus >= 202301L && \
    defined(__cpp_lib_unreachable)
  #include <utility>
  #define UNREACHABLE std::unreachable()
#elif defined(__GNUC__) || \
      __has_builtin(__builtin_unreachable)
  #define UNREACHABLE __builtin_unreachable()
#elif defined(_MSC_VER)
  #define UNREACHABLE __assume(0)
#else
  #define UNREACHABLE
#endif

#endif
