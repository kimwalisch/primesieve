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

#include <stdint.h>
#include <algorithm>
#include <cmath>
#include <limits>

namespace primesieve {

/// Generate a vector with Möbius function values.
/// This implementation is based on code by Rick Sladkey:
/// https://mathoverflow.net/q/99545
///
Vector<int32_t> generate_moebius(int64_t max)
{
  int64_t sqrt = isqrt(max);
  int64_t size = max + 1;
  Vector<int32_t> mu(size);
  std::fill(mu.begin(), mu.end(), 1);

  for (int64_t i = 2; i <= sqrt; i++)
  {
    if (mu[i] == 1)
    {
      for (int64_t j = i; j < size; j += i)
        mu[j] *= (int32_t) -i;
      for (int64_t j = i * i; j < size; j += i * i)
        mu[j] = 0;
    }
  }

  for (int64_t i = 2; i < size; i++)
  {
    if (mu[i] == i)
      mu[i] = 1;
    else if (mu[i] == -i)
      mu[i] = -1;
    else if (mu[i] < 0)
      mu[i] = 1;
    else if (mu[i] > 0)
      mu[i] = -1;
  }

  return mu;
}

} // namespace

namespace {

/// Calculate the logarithmic integral using
/// Ramanujan's formula:
/// https://en.wikipedia.org/wiki/Logarithmic_integral_function#Series_representation
///
long double li(long double x)
{
  if (x <= 1)
    return 0;

  long double gamma = 0.577215664901532860606512090082402431L;
  long double sum = 0;
  long double inner_sum = 0;
  long double factorial = 1;
  long double p = -1;
  long double q = 0;
  long double power2 = 1;
  long double logx = std::log(x);
  int k = 0;

  for (int n = 1; true; n++)
  {
    p *= -logx;
    factorial *= n;
    q = factorial * power2;
    power2 *= 2;

    for (; k <= (n - 1) / 2; k++)
      inner_sum += 1.0L / (2 * k + 1);

    auto old_sum = sum;
    sum += (p / q) * inner_sum;

    // Not converging anymore
    if (std::abs(sum - old_sum) < std::numeric_limits<long double>::epsilon())
      break;
  }

  return gamma + std::log(logx) + std::sqrt(x) * sum;
}

/// Calculate the offset logarithmic integral which is a very
/// accurate approximation of the number of primes <= x.
/// Li(x) > pi(x) for 24 <= x <= ~ 10^316
///
long double Li(long double x)
{
  long double li2 = 1.045163780117492784844588889194613136L;

  if (x <= li2)
    return 0;
  else
    return li(x) - li2;
}

/// Calculate the inverse offset logarithmic integral which
/// is a very accurate approximation of the nth prime.
/// Li^-1(x) < nth_prime(x) for 7 <= x <= 10^316
///
/// This implementation computes Li^-1(x) as the zero of the
/// function f(z) = Li(z) - x using the Newton–Raphson method.
/// Note that Li'(z) = 1 / log(z).
/// https://math.stackexchange.com/a/853192
///
/// Newton–Raphson method:
/// zn+1 = zn - (f(zn) / f'(zn)).
/// zn+1 = zn - (Li(zn) - x) / (1 / log(zn))
/// zn+1 = zn - (Li(zn) - x) * log(zn)
///
long double Li_inverse(long double x)
{
  if (x < 2)
    return 0;

  long double t = x * std::log(x);
  long double old_term = std::numeric_limits<long double>::infinity();

  while (true)
  {
    long double term = (Li(t) - x) * std::log(t);

    // Not converging anymore
    if (std::abs(term) >= std::abs(old_term))
      break;

    t -= term;
    old_term = term;
  }

  return t;
}

/// Calculate the Riemann R function which is a very accurate
/// approximation of the number of primes below x.
/// RiemannR(x) = \sum_{n=1}^{∞} μ(n)/n * li(x^(1/n))
/// http://mathworld.wolfram.com/RiemannPrimeCountingFunction.html
///
long double Ri(long double x)
{
  if (x <= 1)
    return 0;

  long double sum = 0;
  long double old_term = std::numeric_limits<long double>::infinity();
  auto terms = (int) (std::log2(x) * 2 + 10);
  auto mu = primesieve::generate_moebius(terms);

  for (int n = 1; n < terms; n++)
  {
    if (mu[n])
    {
      long double root = std::pow(x, 1.0L / n);
      long double term = (li(root) * mu[n]) / n;

      // Not converging anymore
      if (std::abs(term) >= std::abs(old_term))
        break;

      sum += term;
      old_term = term;
    }
  }

  return sum;
}

/// Calculate the inverse Riemann R function which is a very
/// accurate approximation of the nth prime.
/// This implementation computes Ri^-1(x) as the zero of the
/// function f(z) = Ri(z) - x using the Newton–Raphson method.
/// Note that Ri'(z) = 1 / log(z).
/// https://math.stackexchange.com/a/853192
///
/// Newton–Raphson method:
/// zn+1 = zn - (f(zn) / f'(zn)).
/// zn+1 = zn - (Ri(zn) - x) / (1 / log(zn))
/// zn+1 = zn - (Ri(zn) - x) * log(zn)
///
long double Ri_inverse(long double x)
{
  if (x < 2)
    return 0;

  long double t = Li_inverse(x);
  long double old_term = std::numeric_limits<long double>::infinity();

  while (true)
  {
    long double term = (Ri(t) - x) * std::log(t);

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

uint64_t Li(uint64_t x)
{
  return (uint64_t) ::Li((long double) x);
}

uint64_t Ri(uint64_t x)
{
  return (uint64_t) ::Ri((long double) x);
}

uint64_t Li_inverse(uint64_t x)
{
  auto res = ::Li_inverse((long double) x);

  // Prevent 64-bit integer overflow
  if (res > (long double) std::numeric_limits<uint64_t>::max())
    return std::numeric_limits<uint64_t>::max();

  return (uint64_t) res;
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
/// this use case primeCountApprox() should be used.
///
uint64_t primePiApprox(uint64_t x)
{
  // Li(x) is faster but less accurate than Ri(x).
  // For small x speed is more important than accuracy.
  if (x < 1e10)
    return Li(x);
  else
    return Ri(x);
}

/// nthPrimeApprox(n) is a very accurate approximation of the nth
/// prime with |nth prime - nthPrimeApprox(n)| < sqrt(nth prime).
/// Please note that nthPrimeApprox(n) may be smaller or larger than
/// the actual nth prime.
///
uint64_t nthPrimeApprox(uint64_t n)
{
  // Li_inverse(n) is faster but less accurate than Ri_inverse(n).
  // For small n speed is more important than accuracy.
  if (n < 1e8)
    return Li_inverse(n);
  else
    return Ri_inverse(n);
}

} // namespace
