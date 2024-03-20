///
/// @file   RiemannR.cpp
/// @brief  This file contains an implementation of the Riemann R
///         function which is a very accurate approximation of PrimePi(x).
///         The accuracy of this implementation depends on width of the
///         long double type. If the long double type has 80 bits (e.g.
///         Linux) then RiemannR(x) is accurate up to 1e15, if the long
///         double type has 64 bits (e.g. MSVC & macOS) then RiemannR(x)
///         is accurate up to 1e12. This implementation could be made
///         more accurate using the non standard __float128 type, but for
///         primesieve's purpose speed is more important than accuracy.
///
///         More details about this Riemann R function implementation:
///         https://github.com/kimwalisch/primesieve/pull/144
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

/// Precomputed values of the Riemann Zeta function.
/// Used in the calculation of the Riemann R function and its derivative.
/// These values are calculated up to a precision of 128 bits.
/// Mathematica: Table[NumberForm[SetPrecision[Zeta[k], 45], {40, 39}], {k, 0, 127}]
///
const primesieve::Array<long double, 128> zeta =
{
 -0.500000000000000000000000000000000000000L,
  std::numeric_limits<long double>::infinity(),
  1.644934066848226436472415166646025189219L,
  1.202056903159594285399738161511449990765L,
  1.082323233711138191516003696541167902775L,
  1.036927755143369926331365486457034168057L,
  1.017343061984449139714517929790920527902L,
  1.008349277381922826839797549849796759600L,
  1.004077356197944339378685238508652465259L,
  1.002008392826082214417852769232412060486L,
  1.000994575127818085337145958900319017006L,
  1.000494188604119464558702282526469936469L,
  1.000246086553308048298637998047739670960L,
  1.000122713347578489146751836526357395714L,
  1.000061248135058704829258545105135333747L,
  1.000030588236307020493551728510645062588L,
  1.000015282259408651871732571487636722023L,
  1.000007637197637899762273600293563029213L,
  1.000003817293264999839856461644621939730L,
  1.000001908212716553938925656957795101353L,
  1.000000953962033872796113152038683449346L,
  1.000000476932986787806463116719604373046L,
  1.000000238450502727732990003648186752995L,
  1.000000119219925965311073067788718882326L,
  1.000000059608189051259479612440207935801L,
  1.000000029803503514652280186063705069366L,
  1.000000014901554828365041234658506630699L,
  1.000000007450711789835429491981004170604L,
  1.000000003725334024788457054819204018402L,
  1.000000001862659723513049006403909945417L,
  1.000000000931327432419668182871764735021L,
  1.000000000465662906503378407298923325122L,
  1.000000000232831183367650549200145597594L,
  1.000000000116415501727005197759297383546L,
  1.000000000058207720879027008892436859891L,
  1.000000000029103850444970996869294252279L,
  1.000000000014551921891041984235929632245L,
  1.000000000007275959835057481014520869012L,
  1.000000000003637979547378651190237236356L,
  1.000000000001818989650307065947584832101L,
  1.000000000000909494784026388928253311839L,
  1.000000000000454747378304215402679911203L,
  1.000000000000227373684582465251522682158L,
  1.000000000000113686840768022784934910484L,
  1.000000000000056843419876275856092771830L,
  1.000000000000028421709768893018554550737L,
  1.000000000000014210854828031606769834307L,
  1.000000000000007105427395210852712877354L,
  1.000000000000003552713691337113673298470L,
  1.000000000000001776356843579120327473349L,
  1.000000000000000888178421093081590309609L,
  1.000000000000000444089210314381336419777L,
  1.000000000000000222044605079804198399932L,
  1.000000000000000111022302514106613372054L,
  1.000000000000000055511151248454812437237L,
  1.000000000000000027755575621361241725816L,
  1.000000000000000013877787809725232762839L,
  1.000000000000000006938893904544153697446L,
  1.000000000000000003469446952165922624744L,
  1.000000000000000001734723476047576572049L,
  1.000000000000000000867361738011993372834L,
  1.000000000000000000433680869002065048750L,
  1.000000000000000000216840434499721978501L,
  1.000000000000000000108420217249424140630L,
  1.000000000000000000054210108624566454109L,
  1.000000000000000000027105054312234688320L,
  1.000000000000000000013552527156101164581L,
  1.000000000000000000006776263578045189098L,
  1.000000000000000000003388131789020796818L,
  1.000000000000000000001694065894509799165L,
  1.000000000000000000000847032947254699835L,
  1.000000000000000000000423516473627283335L,
  1.000000000000000000000211758236813619473L,
  1.000000000000000000000105879118406802339L,
  1.000000000000000000000052939559203398703L,
  1.000000000000000000000026469779601698530L,
  1.000000000000000000000013234889800848991L,
  1.000000000000000000000006617444900424404L,
  1.000000000000000000000003308722450212172L,
  1.000000000000000000000001654361225106076L,
  1.000000000000000000000000827180612553034L,
  1.000000000000000000000000413590306276516L,
  1.000000000000000000000000206795153138258L,
  1.000000000000000000000000103397576569129L,
  1.000000000000000000000000051698788284564L,
  1.000000000000000000000000025849394142282L,
  1.000000000000000000000000012924697071141L,
  1.000000000000000000000000006462348535571L,
  1.000000000000000000000000003231174267785L,
  1.000000000000000000000000001615587133893L,
  1.000000000000000000000000000807793566946L,
  1.000000000000000000000000000403896783473L,
  1.000000000000000000000000000201948391737L,
  1.000000000000000000000000000100974195868L,
  1.000000000000000000000000000050487097934L,
  1.000000000000000000000000000025243548967L,
  1.000000000000000000000000000012621774484L,
  1.000000000000000000000000000006310887242L,
  1.000000000000000000000000000003155443621L,
  1.000000000000000000000000000001577721810L,
  1.000000000000000000000000000000788860905L,
  1.000000000000000000000000000000394430453L,
  1.000000000000000000000000000000197215226L,
  1.000000000000000000000000000000098607613L,
  1.000000000000000000000000000000049303807L,
  1.000000000000000000000000000000024651903L,
  1.000000000000000000000000000000012325952L,
  1.000000000000000000000000000000006162976L,
  1.000000000000000000000000000000003081488L,
  1.000000000000000000000000000000001540744L,
  1.000000000000000000000000000000000770372L,
  1.000000000000000000000000000000000385186L,
  1.000000000000000000000000000000000192593L,
  1.000000000000000000000000000000000096296L,
  1.000000000000000000000000000000000048148L,
  1.000000000000000000000000000000000024074L,
  1.000000000000000000000000000000000012037L,
  1.000000000000000000000000000000000006019L,
  1.000000000000000000000000000000000003009L,
  1.000000000000000000000000000000000001505L,
  1.000000000000000000000000000000000000752L,
  1.000000000000000000000000000000000000376L,
  1.000000000000000000000000000000000000188L,
  1.000000000000000000000000000000000000094L,
  1.000000000000000000000000000000000000047L,
  1.000000000000000000000000000000000000024L,
  1.000000000000000000000000000000000000012L,
  1.000000000000000000000000000000000000006L
};

/// Calculate an initial nth prime approximation using Cesàro's formula.
/// Cesàro, Ernesto (1894). "Sur une formule empirique de M. Pervouchine". Comptes
/// Rendus Hebdomadaires des Séances de l'Académie des Sciences. 119: 848–849.
/// https://en.wikipedia.org/wiki/Prime_number_theorem#Approximations_for_the_nth_prime_number
///
template <typename T>
T initialNthPrimeApprox(T x)
{
  if (x < 1)
    return 0;
  else if (x >= 1 && x < 2)
    return 2;
  else if (x >= 2 && x < 3)
    return 3;

  T logx = std::log(x);
  T loglogx = std::log(logx);
  T t = logx + T(0.5) * loglogx;

  if (x > 1600)
    t += T(0.5) * loglogx - 1 + (loglogx - 2) / logx;
  if (x > 1200000)
    t -= (loglogx * loglogx - 6 * loglogx + 11) / (2 * logx * logx);

  return x * t;
}

/// Calculate the Riemann R function which is a very accurate
/// approximation of the number of primes below x.
/// http://mathworld.wolfram.com/RiemannPrimeCountingFunction.html
/// The calculation is done with the Gram series:
/// RiemannR(x) = 1 + \sum_{k=1}^{∞} ln(x)^k / (zeta(k + 1) * k * k!)
///
template <typename T>
T RiemannR(T x)
{
  if (x < T(0.1))
    return 0;

  T epsilon = std::numeric_limits<T>::epsilon();
  T sum = 1;
  T term = 1;
  T logx = std::log(x);

  // The condition k < ITERS is required in case the computation
  // does not converge. This happened on Linux i386 where
  // the precision of the libc math functions is very limited.
  for (unsigned k = 1; k < 1000; k++)
  {
    term *= logx / k;
    T old_sum = sum;

    if (k + 1 < zeta.size())
      sum += term / (T(zeta[k + 1]) * k);
    else
      // For k >= 127, approximate zeta(k + 1) by 1
      sum += term / k;

    // Not converging anymore
    if (std::abs(sum - old_sum) <= epsilon)
      break;
  }

  return sum;
}

/// Calculate the inverse Riemann R function which is a very
/// accurate approximation of the nth prime.
/// This implementation computes RiemannR^-1(x) = t as the zero of the
/// function f(t) = RiemannR(t) - x using the Newton–Raphson method.
/// https://en.wikipedia.org/wiki/Newton%27s_method
///
template <typename T>
T RiemannR_inverse(T x)
{
  T t = initialNthPrimeApprox(x);
  T old_term = std::numeric_limits<T>::infinity();

  if (x < 3)
    return t;

  // The condition i < ITERS is required in case the computation
  // does not converge. This happened on Linux i386 where
  // the precision of the libc math functions is very limited.
  for (int i = 0; i < 100; i++)
  {
    // term = f(t) / f'(t)
    // f(t) = RiemannR(t) - x
    // RiemannR(t) ~ li(t), with li'(t) = 1 / log(t)
    // term = (RiemannR(t) - x) / li'(t) = (RiemannR(t) - x) * log(t)
    T term = (RiemannR(t) - x) * std::log(t);

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

long double RiemannR(long double x)
{
  if (x <= 100)
    return ::RiemannR((float) x);
  if (x <= 1e8)
    return ::RiemannR((double) x);
  else
    return ::RiemannR((long double) x);
}

long double RiemannR_inverse(long double x)
{
  if (x <= 100)
    return ::RiemannR_inverse((float) x);
  if (x <= 1e8)
    return ::RiemannR_inverse((double) x);
  else
    return ::RiemannR_inverse((long double) x);
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
