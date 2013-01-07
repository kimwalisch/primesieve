///
/// @file  popcount.h
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the New BSD License. See the
/// LICENSE file in the top level directory.
///

#ifndef POPCOUNT_PRIMESIEVE_H
#define POPCOUNT_PRIMESIEVE_H

#include <stdint.h>

namespace soe {

uint64_t popcount(const uint64_t* array, uint64_t size);

}

#endif
