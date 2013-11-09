///
/// @file   Callback.hpp
/// @brief  Callback interface classes.
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

/// Internal class.
class None { };

/// callback interface class. Objects derived from this class can be
/// passed to the primesieve::generate_primes() functions.
/// @param T  must be uint64_t.
///
template <typename T, typename T2 = None>
class Callback
{
public:
  virtual void callback(T prime) = 0;
  virtual ~Callback() { }
};

/// Parallel callback interface class. Objects derived from this
/// class can be passed to the primesieve::parallel_generate_primes()
/// functions.
///
template <>
class Callback<uint64_t, int>
{
public:
  virtual void callback(uint64_t prime, int thread_num) = 0;
  virtual ~Callback() { }
};

} // namespace primesieve

#endif
