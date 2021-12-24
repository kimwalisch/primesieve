///
/// @file   PrimeGenerator.cpp
/// @brief  Generates the primes inside [start, stop] and stores them
///         in a vector. After the primes have been stored in the
///         vector primesieve::iterator iterates over the vector and
///         returns the primes. When there are no more primes left in
///         the vector PrimeGenerator generates new primes.
///
/// Copyright (C) 2019 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/Erat.hpp>
#include <primesieve/forward.hpp>
#include <primesieve/littleendian_cast.hpp>
#include <primesieve/PreSieve.hpp>
#include <primesieve/PrimeGenerator.hpp>
#include <primesieve/pmath.hpp>
#include <primesieve/SievingPrimes.hpp>

#include <stdint.h>
#include <algorithm>
#include <array>
#include <cassert>
#include <vector>

using namespace std;

uint64_t popcount64(uint64_t x);

namespace {

/// First 128 primes
const array<uint64_t, 128> smallPrimes =
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
const array<uint8_t, 720> primePi =
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

PrimeGenerator::PrimeGenerator(uint64_t start, uint64_t stop) :
  Erat(start, stop)
{ }

/// Used by iterator::prev_prime()
void PrimeGenerator::init(vector<uint64_t>& primes)
{
  size_t size = primeCountApprox(start_, stop_);
  primes.reserve(size);

  if (start_ <= maxCachedPrime())
  {
    size_t a = getStartIdx();
    size_t b = getStopIdx();

    primes.insert(primes.end(),
             smallPrimes.begin() + a,
             smallPrimes.begin() + b);
  }

  initErat();
}

/// Used by iterator::next_prime()
void PrimeGenerator::init(vector<uint64_t>& primes, size_t* size)
{
  if (start_ <= maxCachedPrime())
  {
    size_t a = getStartIdx();
    size_t b = getStopIdx();

    *size = b - a;
    assert(*size <= primes.size());

    copy(smallPrimes.begin() + a,
         smallPrimes.begin() + b,
         primes.begin());
  }

  initErat();
}

void PrimeGenerator::initErat()
{
  uint64_t startErat = maxCachedPrime() + 1;
  startErat = max(startErat, start_);
  isInit_ = true;

  if (startErat <= stop_)
  {
    int sieveSize = get_sieve_size();
    Erat::init(startErat, stop_, sieveSize, preSieve_);
    sievingPrimes_.init(this, preSieve_);
  }
}

uint64_t PrimeGenerator::maxCachedPrime()
{
  return smallPrimes.back();
}

size_t PrimeGenerator::getStartIdx() const
{
  size_t startIdx = 0;

  if (start_ > 1)
    startIdx = primePi[start_ - 1];

  return startIdx;
}

size_t PrimeGenerator::getStopIdx() const
{
  size_t stopIdx = 0;

  if (stop_ < maxCachedPrime())
    stopIdx = primePi[stop_];
  else
    stopIdx = smallPrimes.size();

  return stopIdx;
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
bool PrimeGenerator::sieveSegment(vector<uint64_t>& primes)
{
  if (!isInit_)
    init(primes);

  if (hasNextSegment())
  {
    sieveSegment();
    return true;
  }

  return false;
}

/// Used by iterator::next_prime()
bool PrimeGenerator::sieveSegment(vector<uint64_t>& primes, size_t* size)
{
  *size = 0;

  if (!isInit_)
  {
    init(primes, size);
    if (*size > 0)
      return false;
  }

  if (hasNextSegment())
  {
    sieveSegment();
    return true;
  }

  // primesieve only supports primes < 2^64. In case the next
  // prime would be > 2^64 we simply return UINT64_MAX.
  if (stop_ >= numeric_limits<uint64_t>::max())
  {
    primes[0] = ~0ull;
    *size = 1;
  }

  return false;
}

/// This method is used by iterator::prev_prime().
/// This method stores all primes inside [a, b] into the primes
/// vector. (b - a) is about sqrt(stop) so the memory usage is
/// quite large. Also after primesieve::iterator has iterated
/// over the primes inside [a, b] we need to generate new
/// primes which incurs an initialization overhead of O(sqrt(n)).
///
void PrimeGenerator::fill(vector<uint64_t>& primes)
{
  while (sieveSegment(primes))
  {
    while (sieveIdx_ < sieveSize_)
    {
      uint64_t bits = littleendian_cast<uint64_t>(&sieve_[sieveIdx_]);

      for (; bits != 0; bits &= bits - 1)
        primes.push_back(nextPrime(bits, low_));

      low_ += 8 * 30;
      sieveIdx_ += 8;
    }
  }
}

/// This method is used by iterator::next_prime().
/// This method stores only the next few primes (~ 200) in the
/// primes vector. Also for iterator::next_prime() there is no
/// recurring initialization overhead (unlike prev_prime()) for
/// this reason iterator::next_prime() runs up to 2x faster
/// than iterator::prev_prime().
///
void PrimeGenerator::fill(vector<uint64_t>& primes,
                          size_t* size)
{
  do
  {
    if (sieveIdx_ >= sieveSize_)
      if (!sieveSegment(primes, size))
        return;

    // Use local variables to prevent the compiler from
    // writing temporary results to memory.
    size_t i = 0;
    size_t maxSize = primes.size();
    assert(maxSize >= 64);
    uint64_t low = low_;
    uint8_t* sieve = sieve_;
    uint64_t sieveIdx = sieveIdx_;
    uint64_t sieveSize = sieveSize_;

    // Fill the buffer with at least (maxSize - 64) primes.
    // Each loop iteration can generate up to 64 primes
    // so we have to stop generating primes once there is
    // not enough space for 64 more primes.
    do
    {
      uint64_t bits = littleendian_cast<uint64_t>(&sieve[sieveIdx]);

      size_t j = i;
      i += popcount64(bits);

      // Note that bits may become 0 in the middle of the loop, and
      // then we'd continue writing garbage entries to primes[] until
      // the end of the loop is reached. This is not a problem: the
      // value of i is incremented correctly above, the test "i <=
      // maxSize - 64" below guarantees that primes[] has enough
      // space.
      do {
        primes[j+ 0] = nextPrime(bits, low); bits &= bits - 1;
        primes[j+ 1] = nextPrime(bits, low); bits &= bits - 1;
        primes[j+ 2] = nextPrime(bits, low); bits &= bits - 1;
        primes[j+ 3] = nextPrime(bits, low); bits &= bits - 1;
        primes[j+ 4] = nextPrime(bits, low); bits &= bits - 1;
        primes[j+ 5] = nextPrime(bits, low); bits &= bits - 1;
        primes[j+ 6] = nextPrime(bits, low); bits &= bits - 1;
        primes[j+ 7] = nextPrime(bits, low); bits &= bits - 1;
        primes[j+ 8] = nextPrime(bits, low); bits &= bits - 1;
        primes[j+ 9] = nextPrime(bits, low); bits &= bits - 1;
        primes[j+10] = nextPrime(bits, low); bits &= bits - 1;
        primes[j+11] = nextPrime(bits, low); bits &= bits - 1;
        primes[j+12] = nextPrime(bits, low); bits &= bits - 1;
        primes[j+13] = nextPrime(bits, low); bits &= bits - 1;
        primes[j+14] = nextPrime(bits, low); bits &= bits - 1;
        primes[j+15] = nextPrime(bits, low); bits &= bits - 1;
        j += 16;
      } while (j < i);

      low += 8 * 30;
      sieveIdx += 8;
    }
    while (i <= maxSize - 64 &&
           sieveIdx < sieveSize);

    low_ = low;
    sieveIdx_ = sieveIdx;
    *size = i;
  }
  while (*size == 0);
}

} // namespace
