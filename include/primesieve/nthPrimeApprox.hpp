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

uint64_t RiemannR(uint64_t x);
uint64_t RiemannR_inverse(uint64_t x);
uint64_t primePiApprox(uint64_t x);
uint64_t nthPrimeApprox(uint64_t n);

} // namespace
