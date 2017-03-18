///
/// @file   Callback.hpp
/// @brief  Callback interface class.
///
/// Copyright (C) 2016 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef CALLBACK_PRIMESIEVE_HPP
#define CALLBACK_PRIMESIEVE_HPP

#include <stdint.h>

namespace primesieve {

class Callback
{
public:
  virtual void callback(uint64_t prime) = 0;
  virtual ~Callback() { }
};

} // namespace

#endif
