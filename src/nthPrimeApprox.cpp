///
/// @file  nthPrimeApprox.cpp
///        This file contains implementations of the logarithmic
///        integral and the Riemann R function which are very
///        accurate approximations of PrimePi(x). Please note that
///        most of this code has been copied from the primecount
///        project: https://github.com/kimwalisch/primecount
///
///        Note that while the Riemann R function is extremely
///        accurate it is much slower than other simpler PrimePi(x)
///        approximations. When speed matters, e.g. for allocating
///        a vector of primes, we avoid using the functions defined
///        in this file. Currently, the functions defined in this
///        file are only used in nthPrime.cpp where accuracy is of
///        utmost importance.
///
/// Copyright (C) 2024 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/pmath.hpp>
#include <primesieve/Vector.hpp>
#include <primesieve/forward.hpp>

#include <stdint.h>
#include <algorithm>
#include <cmath>
#include <limits>

namespace {

/// Calculate the Riemann R function which is a very accurate
/// approximation of the number of primes below x.
/// http://mathworld.wolfram.com/RiemannPrimeCountingFunction.html
/// The calculation is done with the Gram series:
/// RiemannR(x) = 1 + \sum_{k=1}^{∞} ln(x)^k / (zeta(k + 1) * k * k!)
///
long double Ri(long double x)
{
  if (x < 0.1)
    return 0;

  long double epsilon = std::numeric_limits<long double>::epsilon();
  long double old_sum = -1;
  long double sum = 1;
  long double term = 1;
  long double logx = std::log(x);

  for (int k = 1; k < 128 && std::abs(old_sum - sum) >= epsilon; k++)
  {
    long double k_inv = 1.0L / (long double)k;
    term *= logx * k_inv;
    old_sum = sum;
    sum += term * k_inv * primesieve::zetaInv[k];
  }

  // For k >= 128, approximate zeta(k + 1) by 1. 
  for (int k = 128; std::abs(old_sum - sum) >= epsilon; k++)
  {
    long double k_inv = 1.0L / (long double)k;
    term *= logx * k_inv;
    old_sum = sum;
    sum += term * k_inv;
  }

  return sum;
}

/// Calculate the derivative of the Riemann R function.
/// RiemannR'(x) = 1/x * \sum_{k=1}^{∞} ln(x)^(k-1) / (zeta(k + 1) * k!)
///
long double Ri_prime(long double x)
{
  if (x < 0.1)
    return 0;

  long double epsilon = std::numeric_limits<long double>::epsilon();
  long double old_sum = -1;
  long double sum = 0;
  long double term = 1;
  long double logx = std::log(x);

  for (int k = 1; k < 128 && std::abs(old_sum - sum) >= epsilon; k++)
  {
    long double k_inv = 1.0L / (long double)k;
    term *= logx * k_inv;
    old_sum = sum;
    sum += term * primesieve::zetaInv[k];
  }

  // For k >= 128, approximate zeta(k + 1) by 1. 
  for (int k = 128; std::abs(old_sum - sum) >= epsilon; k++)
  {
    long double k_inv = 1.0L / (long double)k;
    term *= logx * k_inv;
    old_sum = sum;
    sum += term;
  }

  return sum / (x * logx);
}

/// Calculate the inverse Riemann R function which is a very
/// accurate approximation of the nth prime.
/// This implementation computes Ri^-1(x) as the zero of the
/// function f(z) = Ri(z) - x using the Newton–Raphson method.
/// https://math.stackexchange.com/a/853192
///
/// Newton–Raphson method:
/// zn+1 = zn - (f(zn) / f'(zn)).
/// zn+1 = zn - (Ri(zn) - x) / Ri'(zn)
///
long double Ri_inverse(long double x)
{
  if (x < 2)
    return 0;

  long double logx = std::log(x);
  long double loglogx = std::log(logx);

  // Calculate an initial approximation for the inverse
  long double t = logx + 0.5L * loglogx;
  if (x > 1600)
    t += 0.5L * loglogx - 1.0L + (loglogx - 2.0L) / logx;
  if (x > 1200000)
    t -= (loglogx * loglogx - 6.0L * loglogx + 11.0L) / (2.0L * logx * logx);
  t *= x;

  long double old_term = std::numeric_limits<long double>::infinity();

  while (true)
  {
    long double term = (Ri(t) - x) / Ri_prime(t);

    // Not converging anymore
    if (std::abs(term) >= std::abs(old_term))
      break;

    t -= term;
    old_term = term;
  }

  return t;
}

} // namespace

namespace primesieve {

uint64_t Ri(uint64_t x)
{
  return (uint64_t) ::Ri((long double) x);
}

uint64_t Ri_inverse(uint64_t x)
{
  auto res = ::Ri_inverse((long double) x);

  // Prevent 64-bit integer overflow
  if (res > (long double) std::numeric_limits<uint64_t>::max())
    return std::numeric_limits<uint64_t>::max();

  return (uint64_t) res;
}

/// primePiApprox(x) is a very accurate approximation of PrimePi(x)
/// with |PrimePi(x) - primePiApprox(x)| < sqrt(x).
/// Since primePiApprox(x) may be smaller than PrimePi(x) it
/// cannot be used to calculate the size of a primes array, for
/// this use case primeCountUpper() should be used.
///
uint64_t primePiApprox(uint64_t x)
{
  return Ri(x);
}

/// nthPrimeApprox(n) is a very accurate approximation of the nth
/// prime with |nth prime - nthPrimeApprox(n)| < sqrt(nth prime).
/// Please note that nthPrimeApprox(n) may be smaller or larger than
/// the actual nth prime.
///
uint64_t nthPrimeApprox(uint64_t n)
{
  return Ri_inverse(n);
}

} // namespace
