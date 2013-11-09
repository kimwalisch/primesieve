///
/// @file   Callback.hpp
/// @brief  Callback interface class for PrimeSieve and
///         ParallelPrimeSieve objects.
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef CALLBACK_PRIMESIEVE_HPP
#define CALLBACK_PRIMESIEVE_HPP

#include <stdint.h>

namespace primesieve {

class None { };

template <typename T1, typename T2 = None>
class Callback
{
public:
  virtual void callback(T1 prime) = 0;
  virtual ~Callback() { }
};

template <>
class Callback<uint64_t, int>
{
public:
  virtual void callback(uint64_t prime, int thread_num) = 0;
  virtual ~Callback() { }
};

} // namespace primesieve

#endif
