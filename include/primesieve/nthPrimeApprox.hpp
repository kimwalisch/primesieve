///
/// @file  nthPrimeApprox.hpp
///
/// Copyright (C) 2024 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/Vector.hpp>
#include <stdint.h>

namespace primesieve {

Vector<int32_t> generate_moebius(int64_t max);
uint64_t Li(uint64_t x);
uint64_t Li_inverse(uint64_t x);
uint64_t Ri(uint64_t x);
uint64_t Ri_inverse(uint64_t x);
uint64_t primesApprox(uint64_t x);
uint64_t nthPrimeApprox(uint64_t n);

} // namespace
