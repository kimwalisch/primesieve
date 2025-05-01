///
/// @file   PrimeGenerator.cpp
/// @brief  Generates the primes inside [start, stop] and stores them
///         in a vector. After the primes have been stored in the
///         vector primesieve::iterator iterates over the vector and
///         returns the primes. When there are no more primes left in
///         the vector PrimeGenerator generates new primes.
///
///         primesieve::iterator's next_prime() performance depends
///         on PrimeGenerator::fillNextPrimes(). Therefore
///         fillNextPrimes() is highly optimized using hardware
///         acceleration (e.g. CTZ, AVX512) whenever possible.
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
/// Copyright (C) 2022 @zielaj, https://github.com/zielaj
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include "PrimeGenerator.hpp"
#include "Erat.hpp"
#include "SievingPrimes.hpp"

#include <primesieve/forward.hpp>
#include <primesieve/littleendian_cast.hpp>
#include <primesieve/macros.hpp>
#include <primesieve/primesieve_error.hpp>
#include <primesieve/macros.hpp>
#include <primesieve/pmath.hpp>
#include <primesieve/popcnt.hpp>
#include <primesieve/Vector.hpp>

#include <stdint.h>
#include <algorithm>
#include <limits>

namespace {

/// First 128 primes
const primesieve::Array<uint64_t, 128> smallPrimes =
{
    2,   3,   5,   7,  11,  13,  17,  19,  23,  29,
   31,  37,  41,  43,  47,  53,  59,  61,  67,  71,
   73,  79,  83,  89,  97, 101, 103, 107, 109, 113,
  127, 131, 137, 139, 149, 151, 157, 163, 167, 173,
  179, 181, 191, 193, 197, 199, 211, 223, 227, 229,
  233, 239, 241, 251, 257, 263, 269, 271, 277, 281,
  283, 293, 307, 311, 313, 317, 331, 337, 347, 349,
  353, 359, 367, 373, 379, 383, 389, 397, 401, 409,
  419, 421, 431, 433, 439, 443, 449, 457, 461, 463,
  467, 479, 487, 491, 499, 503, 509, 521, 523, 541,
  547, 557, 563, 569, 571, 577, 587, 593, 599, 601,
  607, 613, 617, 619, 631, 641, 643, 647, 653, 659,
  661, 673, 677, 683, 691, 701, 709, 719
};

/// Number of primes <= n
const primesieve::Array<uint8_t, 720> primePi =
{
    0,   0,   1,   2,   2,   3,   3,   4,   4,   4,   4,   5,   5,   6,   6,
    6,   6,   7,   7,   8,   8,   8,   8,   9,   9,   9,   9,   9,   9,  10,
   10,  11,  11,  11,  11,  11,  11,  12,  12,  12,  12,  13,  13,  14,  14,
   14,  14,  15,  15,  15,  15,  15,  15,  16,  16,  16,  16,  16,  16,  17,
   17,  18,  18,  18,  18,  18,  18,  19,  19,  19,  19,  20,  20,  21,  21,
   21,  21,  21,  21,  22,  22,  22,  22,  23,  23,  23,  23,  23,  23,  24,
   24,  24,  24,  24,  24,  24,  24,  25,  25,  25,  25,  26,  26,  27,  27,
   27,  27,  28,  28,  29,  29,  29,  29,  30,  30,  30,  30,  30,  30,  30,
   30,  30,  30,  30,  30,  30,  30,  31,  31,  31,  31,  32,  32,  32,  32,
   32,  32,  33,  33,  34,  34,  34,  34,  34,  34,  34,  34,  34,  34,  35,
   35,  36,  36,  36,  36,  36,  36,  37,  37,  37,  37,  37,  37,  38,  38,
   38,  38,  39,  39,  39,  39,  39,  39,  40,  40,  40,  40,  40,  40,  41,
   41,  42,  42,  42,  42,  42,  42,  42,  42,  42,  42,  43,  43,  44,  44,
   44,  44,  45,  45,  46,  46,  46,  46,  46,  46,  46,  46,  46,  46,  46,
   46,  47,  47,  47,  47,  47,  47,  47,  47,  47,  47,  47,  47,  48,  48,
   48,  48,  49,  49,  50,  50,  50,  50,  51,  51,  51,  51,  51,  51,  52,
   52,  53,  53,  53,  53,  53,  53,  53,  53,  53,  53,  54,  54,  54,  54,
   54,  54,  55,  55,  55,  55,  55,  55,  56,  56,  56,  56,  56,  56,  57,
   57,  58,  58,  58,  58,  58,  58,  59,  59,  59,  59,  60,  60,  61,  61,
   61,  61,  61,  61,  61,  61,  61,  61,  62,  62,  62,  62,  62,  62,  62,
   62,  62,  62,  62,  62,  62,  62,  63,  63,  63,  63,  64,  64,  65,  65,
   65,  65,  66,  66,  66,  66,  66,  66,  66,  66,  66,  66,  66,  66,  66,
   66,  67,  67,  67,  67,  67,  67,  68,  68,  68,  68,  68,  68,  68,  68,
   68,  68,  69,  69,  70,  70,  70,  70,  71,  71,  71,  71,  71,  71,  72,
   72,  72,  72,  72,  72,  72,  72,  73,  73,  73,  73,  73,  73,  74,  74,
   74,  74,  74,  74,  75,  75,  75,  75,  76,  76,  76,  76,  76,  76,  77,
   77,  77,  77,  77,  77,  77,  77,  78,  78,  78,  78,  79,  79,  79,  79,
   79,  79,  79,  79,  80,  80,  80,  80,  80,  80,  80,  80,  80,  80,  81,
   81,  82,  82,  82,  82,  82,  82,  82,  82,  82,  82,  83,  83,  84,  84,
   84,  84,  84,  84,  85,  85,  85,  85,  86,  86,  86,  86,  86,  86,  87,
   87,  87,  87,  87,  87,  87,  87,  88,  88,  88,  88,  89,  89,  90,  90,
   90,  90,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  92,
   92,  92,  92,  92,  92,  92,  92,  93,  93,  93,  93,  94,  94,  94,  94,
   94,  94,  94,  94,  95,  95,  95,  95,  96,  96,  96,  96,  96,  96,  97,
   97,  97,  97,  97,  97,  97,  97,  97,  97,  97,  97,  98,  98,  99,  99,
   99,  99,  99,  99,  99,  99,  99,  99,  99,  99,  99,  99,  99,  99,  99,
   99, 100, 100, 100, 100, 100, 100, 101, 101, 101, 101, 101, 101, 101, 101,
  101, 101, 102, 102, 102, 102, 102, 102, 103, 103, 103, 103, 103, 103, 104,
  104, 105, 105, 105, 105, 105, 105, 106, 106, 106, 106, 106, 106, 106, 106,
  106, 106, 107, 107, 107, 107, 107, 107, 108, 108, 108, 108, 108, 108, 109,
  109, 110, 110, 110, 110, 110, 110, 111, 111, 111, 111, 111, 111, 112, 112,
  112, 112, 113, 113, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114,
  114, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 116, 116, 117, 117,
  117, 117, 118, 118, 118, 118, 118, 118, 119, 119, 119, 119, 119, 119, 120,
  120, 121, 121, 121, 121, 121, 121, 121, 121, 121, 121, 121, 121, 122, 122,
  122, 122, 123, 123, 123, 123, 123, 123, 124, 124, 124, 124, 124, 124, 124,
  124, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 126, 126, 126, 126,
  126, 126, 126, 126, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 128
};

} // namespace

namespace primesieve {

PrimeGenerator::PrimeGenerator(uint64_t start,
                               uint64_t stop) :
  Erat(start, stop)
{ }

uint64_t PrimeGenerator::maxCachedPrime()
{
  return smallPrimes.back();
}

std::size_t PrimeGenerator::getStartIdx() const
{
  std::size_t startIdx = 0;

  if (start_ > 1)
    startIdx = primePi[start_ - 1];

  return startIdx;
}

std::size_t PrimeGenerator::getStopIdx() const
{
  std::size_t stopIdx = 0;

  if (stop_ < maxCachedPrime())
    stopIdx = primePi[stop_];
  else
    stopIdx = smallPrimes.size();

  return stopIdx;
}

/// Used by iterator::prev_prime()
void PrimeGenerator::initPrevPrimes(Vector<uint64_t>& primes,
                                    std::size_t* size)
{
  auto resize = [](Vector<uint64_t>& primes,
                   std::size_t size)
  {
    // Avoids reallocation in fillPrevPrimes()
    size += 64;

    if (primes.empty())
      primes.resize(size);
    // When sieving backwards the number of primes inside [start, stop]
    // slowly increases in each new segment as there are more small
    // than large primes. Our new size has been calculated using
    // primeCountUpper(start, stop) which is usually too large by 4%
    // near 10^12 and by 2.5% near 10^19. Hence if the new size is less
    // than 1% larger than the old size we do not increase the primes
    // buffer as it will likely be large enough to fit all primes.
    else if (size > primes.size() &&
             (double) size / (double) primes.size() > 1.01)
    {
      // Prevent unnecessary copying when resizing
      primes.clear();
      primes.resize(size);
    }
  };

  std::size_t pix = primeCountUpper(start_, stop_);

  if (start_ <= maxCachedPrime())
  {
    std::size_t a = getStartIdx();
    std::size_t b = getStopIdx();
    ASSERT(a <= b);

    *size = (start_ <= 2) + b - a;
    resize(primes, std::max(*size, pix));
    std::size_t i = 0;

    if (start_ <= 2)
      primes[i++] = 0;

    std::copy(smallPrimes.begin() + a,
              smallPrimes.begin() + b,
              &primes[i]);
  }
  else
    resize(primes, pix);

  initErat();
}

/// Used by iterator::next_prime()
void PrimeGenerator::initNextPrimes(Vector<uint64_t>& primes,
                                    std::size_t* size)
{
  auto resize = [](Vector<uint64_t>& primes,
                   std::size_t size)
  {
    if (size > primes.size())
    {
      // Prevent unnecessary copying when resizing
      primes.clear();
      primes.resize(size);
    }
  };

  // A buffer of 1024 primes provides good
  // performance with little memory usage.
  std::size_t maxSize = 1 << 10;

  if (start_ <= maxCachedPrime())
  {
    std::size_t a = getStartIdx();
    std::size_t b = getStopIdx();
    *size = b - a;

    if (stop_ < maxCachedPrime() + 2)
      resize(primes, *size);
    else
    {
      // +64 is needed because our fillNextPrimes()
      // algorithm aborts as soon as there is not
      // enough space to store 64 more primes.
      std::size_t minSize = *size + 64;
      std::size_t pix = primeCountUpper(start_, stop_) + 64;
      pix = inBetween(minSize, pix, maxSize);
      pix = std::max(*size, pix);
      resize(primes, pix);
    }

    ASSERT(primes.size() >= *size);
    std::copy(smallPrimes.begin() + a,
              smallPrimes.begin() + b,
              primes.begin());
  }
  else
  {
    // +64 is needed because our fillNextPrimes()
    // algorithm aborts as soon as there is not
    // enough space to store 64 more primes.
    std::size_t minSize = 64;
    std::size_t pix = primeCountUpper(start_, stop_) + 64;
    pix = inBetween(minSize, pix, maxSize);
    resize(primes, pix);
  }

  initErat();
}

void PrimeGenerator::initErat()
{
  ASSERT(maxCachedPrime() >= 5);
  uint64_t startErat = maxCachedPrime() + 2;
  startErat = std::max(startErat, start_);
  isInit_ = true;

  if (startErat <= stop_ &&
      startErat < std::numeric_limits<uint64_t>::max())
  {
    int sieveSize = get_sieve_size();
    Erat::init(startErat, stop_, sieveSize, memoryPool_);
    sievingPrimes_.init(this, sieveSize, memoryPool_);
  }
}

void PrimeGenerator::sieveSegment()
{
  uint64_t sqrtHigh = isqrt(segmentHigh_);

  sieveIdx_ = 0;
  low_ = segmentLow_;

  if (!prime_)
    prime_ = sievingPrimes_.next();

  while (prime_ <= sqrtHigh)
  {
    addSievingPrime(prime_);
    prime_ = sievingPrimes_.next();
  }

  Erat::sieveSegment();
}

/// Used by iterator::prev_prime()
bool PrimeGenerator::sievePrevPrimes(Vector<uint64_t>& primes,
                                     std::size_t* size)
{
  if (!isInit_)
    initPrevPrimes(primes, size);

  if (hasNextSegment())
  {
    sieveSegment();
    return true;
  }

  // We have generated all primes inside [start, stop], we cannot
  // generate more primes using this PrimeGenerator. Therefore we
  // need to allocate a new PrimeGenerator in iterator.cpp.
  return false;
}

/// Used by iterator::next_prime()
bool PrimeGenerator::sieveNextPrimes(Vector<uint64_t>& primes,
                                     std::size_t* size)
{
  if (!isInit_)
    initNextPrimes(primes, size);

  if (hasNextSegment())
  {
    sieveSegment();
    return true;
  }

  // The next prime would be > 2^64
  if_unlikely(stop_ >= std::numeric_limits<uint64_t>::max())
    throw primesieve_error("cannot generate primes > 2^64");

  // We have generated all primes <= stop, we cannot generate
  // more primes using this PrimeGenerator. Therefore we
  // need to allocate a new PrimeGenerator in iterator.cpp.
  return false;
}

} // namespace

#if defined(ENABLE_PRIMEGENERATOR_DEFAULT)
  #include "PrimeGenerator_default.hpp"
#endif

#if defined(ENABLE_AVX512_VBMI2) || \
    defined(ENABLE_MULTIARCH_AVX512_VBMI2)
  #include "PrimeGenerator_x86_avx512.hpp"
#endif
