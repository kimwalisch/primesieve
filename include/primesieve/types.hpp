///
/// @file   types.hpp
/// @brief  Types and forward declarations.
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef TYPES_HPP
#define TYPES_HPP

#include <stdint.h>

namespace primesieve {

/// byte type must be unsigned and have 8-bits
using byte_t = uint8_t;
using uint_t = unsigned int;

int get_num_threads();
int get_sieve_size();

uint64_t get_max_stop();
uint64_t popcount(const uint64_t* array, uint64_t size);

} // namespace

#endif
