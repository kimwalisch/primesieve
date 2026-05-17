///
/// @file   forward.hpp
/// @brief  Forward declarations.
///
/// Copyright (C) 2023 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef TYPES_HPP
#define TYPES_HPP

#include "Vector.hpp"
#include <stdint.h>

namespace primesieve {

extern const Array<uint64_t, 65> bitValues;
extern const Array<uint64_t, 64> bruijnBitValues;

int get_num_threads();
int get_sieve_size();

uint64_t get_max_stop();
uint64_t popcount(const uint64_t* array, uint64_t size);

} // namespace

#endif
