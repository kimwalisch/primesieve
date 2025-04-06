///
/// @file   PreSieve_default.hpp
/// @brief  The presieve*_default() functions are important for
///         performance and therefore it is important that these
///         functions are auto-vectorized by the compiler. If these
///         functions are not auto-vectorized primesieve's
///         performance will deteriorate by up to 30%.
///
///         Clang vectorizes presieve*_default() with -O2 and -O3,
///         which is great. GCC only vectorizes presieve*_default()
///         with -O3 but not with -O2. GCC -O2 uses the very-cheap
///         cost model which prevents our presieve*_default()
///         functions from getting auto vectorized. But compiling with
///         "gcc -O2 -ftree-vectorize -fvect-cost-model=dynamic"
///         fixes this issue. Therefore auto_vectorization.cmake
///         enables the use of the -ftree-vectorize and
///         -fvect-cost-model=dynamic compiler options if they are
///         supported by the compiler.
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
/// Copyright (C) 2022 @zielaj, https://github.com/zielaj
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRESIEVE_DEFAULT_HPP
#define PRESIEVE_DEFAULT_HPP

#include <stdint.h>
#include <cstddef>

namespace {

void presieve1_default(const uint8_t* __restrict preSieved0,
                       const uint8_t* __restrict preSieved1,
                       const uint8_t* __restrict preSieved2,
                       const uint8_t* __restrict preSieved3,
                       uint8_t* __restrict sieve,
                       std::size_t bytes)
{
  for (std::size_t i = 0; i < bytes; i++)
    sieve[i] = preSieved0[i] & preSieved1[i] & preSieved2[i] & preSieved3[i];
}

void presieve2_default(const uint8_t* __restrict preSieved0,
                       const uint8_t* __restrict preSieved1,
                       const uint8_t* __restrict preSieved2,
                       const uint8_t* __restrict preSieved3,
                       uint8_t* __restrict sieve,
                       std::size_t bytes)
{
  for (std::size_t i = 0; i < bytes; i++)
    sieve[i] &= preSieved0[i] & preSieved1[i] & preSieved2[i] & preSieved3[i];
}

} // namespace

#endif
