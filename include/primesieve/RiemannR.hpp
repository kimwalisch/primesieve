///
/// @file  RiemannR.hpp
///
/// Copyright (C) 2024 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <stdint.h>

namespace primesieve {

long double RiemannR(long double x);
long double RiemannR_inverse(long double x);

uint64_t primePiApprox(uint64_t x);
uint64_t nthPrimeApprox(uint64_t n);

} // namespace
