///
/// @file  util.cpp
///        This file contains helper functions.
///
/// Copyright (C) 2026 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#if defined(ENABLE_ASSERT)

#include <primesieve/macros.hpp>

#include <cstdlib>
#include <iostream>
#include <string>

namespace primesieve {

/// Custom assertion failure handler.
/// On MinGW GCC 16, the standard assert() implementation no longer
/// guarantees a non-returning failure path, which can cause false
/// positive compiler warnings such as -Warray-bounds in debug builds.
/// https://github.com/mingw-w64/mingw-w64/commit/ecf2328a328d11dec7044b40b2b5e93b5b2b9d9e
///
/// Using our own [[noreturn]] handler lets the compiler correctly
/// infer that execution cannot continue after a failed assertion.
///
[[noreturn]]
void assert_failed(const char* expression,
                   const char* file,
                   const char* function,
                   int line)
{
  std::string msg("\n");
  msg += std::string(file) + ":" + std::to_string(line);
  msg += ": " + std::string(function);
  msg += ": Assertion failed: `";
  msg += expression + std::string("'\n\n");

  std::cerr << msg;

  std::abort();
}

} // namespace

#endif
