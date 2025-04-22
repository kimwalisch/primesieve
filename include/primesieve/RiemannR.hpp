///
/// @file  RiemannR.hpp
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef RIEMANNR_HPP
#define RIEMANNR_HPP

#include <stdint.h>

namespace primesieve {

long double RiemannR(long double x);
long double RiemannR_inverse(long double x);

uint64_t primePiApprox(uint64_t x);
uint64_t nthPrimeApprox(uint64_t n);

} // namespace

#endif
