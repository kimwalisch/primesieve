///
/// @file  RiemannR.cpp
///        This file contains an implementation of the Riemann R
///        function which is a very accurate approximation of
///        PrimePi(x). Note that this Riemann R implementation is
///        only accurate up to about 10^19 (if the long double type
///        has 80 bits). It could be made more accurate using the
///        non standard __float128 type, but for primesieve's purpose
///        speed is more important than accuracy.
///
///        More details about this Riemann R function implementation:
///        https://github.com/kimwalisch/primesieve/pull/144
///
/// Copyright (C) 2024 Kim Walisch, <kim.walisch@gmail.com>
/// Copyright (C) 2024 @nipzu, https://github.com/nipzu
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/Vector.hpp>

#include <stdint.h>
#include <cmath>
#include <limits>

namespace {

/// Precomputed values of zetaInv[k] = 1 / zeta(k + 1).
/// Used in the calculation of the Riemann R function and its derivative.
/// These values are calculated up to a precision of 128 bits.
/// Mathematica: Table[NumberForm[SetPrecision[1/Zeta[k + 1], 45], {38, 38}], {k, 0, 127}]
///
const primesieve::Array<long double, 128> zetaInv =
{
  0.00000000000000000000000000000000000000L,
  0.60792710185402662866327677925836583343L,
  0.83190737258070746868312627882153073442L,
  0.92393840292159016702375049404068247276L,
  0.96438734042926245912643658844498457124L,
  0.98295259226458041980489656499392413295L,
  0.99171985583844431042818593149755069165L,
  0.99593920112551514683483647280554532401L,
  0.99799563273076215686467613210509999632L,
  0.99900641306903078175222531809290878575L,
  0.99950605549762467678582298009453697739L,
  0.99975397399038468206770164303673058470L,
  0.99987730170913952450133620378723676486L,
  0.99993875561604559519730175829325041150L,
  0.99996941269930456117242340889418930870L,
  0.99998471797413523168338429153076556170L,
  0.99999236286068844254826438672217335846L,
  0.99999618272130667240681034896437598818L,
  0.99999809179092471488437703328457254051L,
  0.99999904603887616989781139055273723226L,
  0.99999952306724067715893767591651988928L,
  0.99999976154955413089570313635363076684L,
  0.99999988078008824807797958727410486624L,
  0.99999994039181450187651056224846508481L,
  0.99999997019649737359651508890102089892L,
  0.99999998509844539369129175912634580635L,
  0.99999999254928826567767626959951349247L,
  0.99999999627466598908965648972675175641L,
  0.99999999813734027995645223273129800534L,
  0.99999999906867256844770260269784066457L,
  0.99999999953433709371346353509327556591L,
  0.99999999976716881668655981073561305389L,
  0.99999999988358449828654737128147523751L,
  0.99999999994179227912436112987729664106L,
  0.99999999997089614955587603724140433426L,
  0.99999999998544807810916977419479027318L,
  0.99999999999272404016499545857700011549L,
  0.99999999999636202045263458370494986087L,
  0.99999999999818101034969624277576308610L,
  0.99999999999909050521597443825250885862L,
  0.99999999999954525262169599139249816326L,
  0.99999999999977262631541758644726975844L,
  0.99999999999988631315923199013976285333L,
  0.99999999999994315658012372737508161140L,
  0.99999999999997157829023110778923903545L,
  0.99999999999998578914517196859517856064L,
  0.99999999999999289457260478919777422111L,
  0.99999999999999644728630866289894847610L,
  0.99999999999999822364315642088282797029L,
  0.99999999999999911182157890691919855130L,
  0.99999999999999955591078968561886079545L,
  0.99999999999999977795539492019585090387L,
  0.99999999999999988897769748589339895390L,
  0.99999999999999994448884875154519064425L,
  0.99999999999999997224442437863875904456L,
  0.99999999999999998612221219027476742975L,
  0.99999999999999999306110609545584635070L,
  0.99999999999999999653055304783407738729L,
  0.99999999999999999826527652395242343096L,
  0.99999999999999999913263826198800662792L,
  0.99999999999999999956631913099793495144L,
  0.99999999999999999978315956550027802155L,
  0.99999999999999999989157978275057585938L,
  0.99999999999999999994578989137543354589L,
  0.99999999999999999997289494568776531168L,
  0.99999999999999999998644747284389883542L,
  0.99999999999999999999322373642195481090L,
  0.99999999999999999999661186821097920318L,
  0.99999999999999999999830593410549020083L,
  0.99999999999999999999915296705274530017L,
  0.99999999999999999999957648352637271667L,
  0.99999999999999999999978824176318638053L,
  0.99999999999999999999989412088159319766L,
  0.99999999999999999999994706044079660130L,
  0.99999999999999999999997353022039830147L,
  0.99999999999999999999998676511019915101L,
  0.99999999999999999999999338255509957560L,
  0.99999999999999999999999669127754978783L,
  0.99999999999999999999999834563877489392L,
  0.99999999999999999999999917281938744697L,
  0.99999999999999999999999958640969372348L,
  0.99999999999999999999999979320484686174L,
  0.99999999999999999999999989660242343087L,
  0.99999999999999999999999994830121171544L,
  0.99999999999999999999999997415060585772L,
  0.99999999999999999999999998707530292886L,
  0.99999999999999999999999999353765146443L,
  0.99999999999999999999999999676882573221L,
  0.99999999999999999999999999838441286611L,
  0.99999999999999999999999999919220643305L,
  0.99999999999999999999999999959610321653L,
  0.99999999999999999999999999979805160826L,
  0.99999999999999999999999999989902580413L,
  0.99999999999999999999999999994951290207L,
  0.99999999999999999999999999997475645103L,
  0.99999999999999999999999999998737822552L,
  0.99999999999999999999999999999368911276L,
  0.99999999999999999999999999999684455638L,
  0.99999999999999999999999999999842227819L,
  0.99999999999999999999999999999921113909L,
  0.99999999999999999999999999999960556955L,
  0.99999999999999999999999999999980278477L,
  0.99999999999999999999999999999990139239L,
  0.99999999999999999999999999999995069619L,
  0.99999999999999999999999999999997534810L,
  0.99999999999999999999999999999998767405L,
  0.99999999999999999999999999999999383702L,
  0.99999999999999999999999999999999691851L,
  0.99999999999999999999999999999999845926L,
  0.99999999999999999999999999999999922963L,
  0.99999999999999999999999999999999961481L,
  0.99999999999999999999999999999999980741L,
  0.99999999999999999999999999999999990370L,
  0.99999999999999999999999999999999995185L,
  0.99999999999999999999999999999999997593L,
  0.99999999999999999999999999999999998796L,
  0.99999999999999999999999999999999999398L,
  0.99999999999999999999999999999999999699L,
  0.99999999999999999999999999999999999850L,
  0.99999999999999999999999999999999999925L,
  0.99999999999999999999999999999999999962L,
  0.99999999999999999999999999999999999981L,
  0.99999999999999999999999999999999999991L,
  0.99999999999999999999999999999999999995L,
  0.99999999999999999999999999999999999998L,
  0.99999999999999999999999999999999999999L,
  0.99999999999999999999999999999999999999L,
  1.00000000000000000000000000000000000000L
};

/// Calculate an initial nth prime approximation using Cesàro's formula.
/// Cesàro, Ernesto (1894). "Sur une formule empirique de M. Pervouchine". Comptes
/// Rendus Hebdomadaires des Séances de l'Académie des Sciences. 119: 848–849.
/// https://en.wikipedia.org/wiki/Prime_number_theorem#Approximations_for_the_nth_prime_number
///
long double initialNthPrimeApprox(long double x)
{
  if (x < 2)
    return 0;

  long double logx = std::log(x);
  long double t = logx;

  if (x > /* e = */ 2.719)
  {
    long double loglogx = std::log(logx);
    t += 0.5 * loglogx;

    if (x > 1600)
      t += 0.5 * loglogx - 1.0 + (loglogx - 2.0) / logx;
    if (x > 1200000)
      t -= (loglogx * loglogx - 6.0 * loglogx + 11.0) / (2.0 * logx * logx);
  }

  return x * t;
}

/// Calculate the derivative of the Riemann R function.
/// RiemannR'(x) = 1/x * \sum_{k=1}^{∞} ln(x)^(k-1) / (zeta(k + 1) * k!)
///
long double RiemannR_prime(long double x)
{
  if (x < 0.1)
    return 0;

  long double sum = 0;
  long double old_sum = -1;
  long double term = 1;
  long double logx = std::log(x);
  long double epsilon = std::numeric_limits<long double>::epsilon();

  for (int k = 1; k < 128 && std::abs(old_sum - sum) >= epsilon; k++)
  {
    long double k_inv = 1.0L / (long double) k;
    term *= logx * k_inv;
    old_sum = sum;
    sum += term * zetaInv[k];
  }

  // For k >= 128, approximate zeta(k + 1) by 1
  for (int k = 128; std::abs(old_sum - sum) >= epsilon; k++)
  {
    long double k_inv = 1.0L / (long double) k;
    term *= logx * k_inv;
    old_sum = sum;
    sum += term;
  }

  return sum / (x * logx);
}



} // namespace

namespace primesieve {

/// Calculate the Riemann R function which is a very accurate
/// approximation of the number of primes below x.
/// http://mathworld.wolfram.com/RiemannPrimeCountingFunction.html
/// The calculation is done with the Gram series:
/// RiemannR(x) = 1 + \sum_{k=1}^{∞} ln(x)^k / (zeta(k + 1) * k * k!)
///
long double RiemannR(long double x)
{
  if (x < 0.1)
    return 0;

  long double epsilon = std::numeric_limits<long double>::epsilon();

  if (std::abs(x - 1.0) < epsilon)
    return 1;

  long double sum = 1;
  long double old_sum = -1;
  long double term = 1;
  long double logx = std::log(x);

  for (int k = 1; k < 128 && std::abs(old_sum - sum) >= epsilon; k++)
  {
    long double k_inv = 1.0L / (long double) k;
    term *= logx * k_inv;
    old_sum = sum;
    sum += term * k_inv * zetaInv[k];
  }

  // For k >= 128, approximate zeta(k + 1) by 1
  for (int k = 128; std::abs(old_sum - sum) >= epsilon; k++)
  {
    long double k_inv = 1.0L / (long double) k;
    term *= logx * k_inv;
    old_sum = sum;
    sum += term * k_inv;
  }

  return sum;
}

/// Calculate the inverse Riemann R function which is a very
/// accurate approximation of the nth prime.
/// This implementation computes RiemannR^-1(x) as the zero of the
/// function f(z) = RiemannR(z) - x using the Newton–Raphson method.
/// https://math.stackexchange.com/a/853192
///
/// Newton–Raphson method:
/// zn+1 = zn - (f(zn) / f'(zn)).
/// zn+1 = zn - (RiemannR(zn) - x) / RiemannR'(zn)
///
long double RiemannR_inverse(long double x)
{
  if (x < 2)
    return 0;

  long double t = initialNthPrimeApprox(x);
  long double old_term = std::numeric_limits<long double>::infinity();

  while (true)
  {
    long double term = (RiemannR(t) - x) / RiemannR_prime(t);

    // Not converging anymore
    if (std::abs(term) >= std::abs(old_term))
      break;

    t -= term;
    old_term = term;
  }

  return t;
}

/// primePiApprox(x) is a very accurate approximation of PrimePi(x)
/// with |PrimePi(x) - primePiApprox(x)| < sqrt(x).
/// primePiApprox(x) is currently only used in nthPrime.cpp where
/// accuracy is more important than speed. primeCountUpper(x) is much
/// faster, but less accurate than primePiApprox(x). For allocating a
/// vector of primes we always prefer using primeCountUpper(x).
///
uint64_t primePiApprox(uint64_t x)
{
  return (uint64_t) RiemannR((long double) x);
}

/// nthPrimeApprox(n) is a very accurate approximation of the nth
/// prime with |nth prime - nthPrimeApprox(n)| < sqrt(nth prime).
/// Please note that nthPrimeApprox(n) may be smaller or larger than
/// the actual nth prime.
///
uint64_t nthPrimeApprox(uint64_t n)
{
  auto res = RiemannR_inverse((long double) n);

  // Prevent 64-bit integer overflow
  if (res > (long double) std::numeric_limits<uint64_t>::max())
    return std::numeric_limits<uint64_t>::max();

  return (uint64_t) res;
}

} // namespace
