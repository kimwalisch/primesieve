///
/// @file PreSieve_default.hpp
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
