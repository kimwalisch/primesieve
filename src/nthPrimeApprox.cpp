///
/// @file  nthPrimeApprox.cpp
///
/// Copyright (C) 2024 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/pmath.hpp>
#include <primesieve/Vector.hpp>

#include <stdint.h>
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

namespace {

using primesieve::Array;

// tinyPrimes[1] = 2, tinyPrimes[2] = 3, ...
const Array<int16_t, 170> tinyPrimes =
{
    0,   2,   3,   5,   7,  11,  13,  17,  19,  23, 
   29,  31,  37,  41,  43,  47,  53,  59,  61,  67, 
   71,  73,  79,  83,  89,  97, 101, 103, 107, 109, 
  113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 
  173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 
  229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 
  281, 283, 293, 307, 311, 313, 317, 331, 337, 347, 
  349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 
  409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 
  463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 
  541, 547, 557, 563, 569, 571, 577, 587, 593, 599, 
  601, 607, 613, 617, 619, 631, 641, 643, 647, 653, 
  659, 661, 673, 677, 683, 691, 701, 709, 719, 727, 
  733, 739, 743, 751, 757, 761, 769, 773, 787, 797, 
  809, 811, 821, 823, 827, 829, 839, 853, 857, 859, 
  863, 877, 881, 883, 887, 907, 911, 919, 929, 937, 
  941, 947, 953, 967, 971, 977, 983, 991, 997, 1009
};

} //namespace

namespace primesieve {

uint64_t Li(uint64_t x)
{
  return (uint64_t) ::Li((long double) x);
}

uint64_t Li_inverse(uint64_t x)
{
  return (uint64_t) ::Li_inverse((long double) x);
}

uint64_t Ri(uint64_t x)
{
  return (uint64_t) ::Ri((long double) x);
}

uint64_t Ri_inverse(uint64_t x)
{
  return (uint64_t) ::Ri_inverse((long double) x);
}

uint64_t primesApprox(uint64_t x)
{
  // Li(x) is faster but less accurate than Ri(x).
  // For small n speed is more important than accuracy.
  if (x < 1e10)
    return Li(x);
  else
    return Ri(x);
}

uint64_t nthPrimeApprox(uint64_t n)
{
  if (n < tinyPrimes.size())
    return tinyPrimes[n];

  // Li_inverse(x) is faster but less accurate than Ri_inverse(x).
  // For small n speed is more important than accuracy.
  if (n < 1e8)
    return Li_inverse(n);
  else
    return Ri_inverse(n);
}

} // namespace