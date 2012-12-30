///
/// @file  toString.h
///
/// Copyright (C) 2012 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the New BSD License. See the
/// LICENSE file in the top level directory.
///

#ifndef TOSTRING_PRIMESIEVE_H
#define TOSTRING_PRIMESIEVE_H

#include <string>
#include <sstream>

namespace soe {

template <typename T>
inline std::string toString(T t)
{
  std::ostringstream oss;
  oss << t;
  return oss.str();
}

} // namespace soe

#endif
