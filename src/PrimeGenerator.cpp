///
/// @file   PrimeGenerator.cpp
/// @brief  Generates the primes inside [start, stop] and stores them
///         in a vector. After the primes have been stored in the
///         vector primesieve::iterator iterates over the vector and
///         returns the primes. When there are no more primes left in
///         the vector PrimeGenerator generates new primes.
///
///         primesieve::iterator's next_prime() performance depends on
///         PrimeGenerator::fill(). Therefore PrimeGenerator::fill()
///         is highly optimized using hardware acceleration (e.g. CTZ,
///         AVX512) whenever possible.
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
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
#include <primesieve/popcnt.hpp>
#include <primesieve/SievingPrimes.hpp>

#include <stdint.h>
#include <algorithm>
#include <array>
#include <cassert>
#include <vector>

/// Enable AVX512 for GCC & Clang if primesieve is compiled using
/// -march=native on an AVX512 capable CPU. Currently the MSVC
/// compiler is not yet supported as it does not support the
/// __AVX512F__, __AVX512BW__, __AVX512VBMI2__ macros and hence it
/// is not possible to know whether we can safely enable AVX512.
///
#if !defined(DISABLE_AVX512) && \
     defined(__AVX512F__) && \
     defined(__AVX512BW__) && \
     defined(__AVX512VBMI2__) && \
     __has_include(<immintrin.h>)
  #define ENABLE_AVX512
  #include <immintrin.h>
#endif

using std::size_t;

namespace {

/// First 128 primes
const std::array<uint64_t, 128> smallPrimes =
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
const std::array<uint8_t, 720> primePi =
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
void PrimeGenerator::init(std::vector<uint64_t>& primes)
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
void PrimeGenerator::init(std::vector<uint64_t>& primes, size_t* size)
{
  if (start_ <= maxCachedPrime())
  {
    size_t a = getStartIdx();
    size_t b = getStopIdx();

    *size = b - a;
    assert(*size <= primes.size());

    std::copy(smallPrimes.begin() + a,
              smallPrimes.begin() + b,
              primes.begin());
  }

  initErat();
}

void PrimeGenerator::initErat()
{
  uint64_t startErat = maxCachedPrime() + 1;
  startErat = std::max(startErat, start_);
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
bool PrimeGenerator::sieveSegment(std::vector<uint64_t>& primes)
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
bool PrimeGenerator::sieveSegment(std::vector<uint64_t>& primes, size_t* size)
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
  if (stop_ >= std::numeric_limits<uint64_t>::max())
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
void PrimeGenerator::fill(std::vector<uint64_t>& primes)
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

#if defined(ENABLE_AVX512)

/// This algorithm converts 1 bits from the sieve array into primes
/// using AVX512. The algorithm is a modified version of the AVX512
/// algorithm which converts 1 bits into bit indexes from:
/// https://branchfree.org/2018/05/22/bits-to-indexes-in-bmi2-and-avx-512
/// https://github.com/kimwalisch/primesieve/pull/109
///
/// Our algorithm is optimized for sparse bitstreams that are
/// distributed relatively evenly. While processing a 64-bit word
/// from the sieve array there are if checks that skip to the next
/// loop iteration once all 1 bits have been processed. In my
/// benchmarks this algorithm ran about 15% faster than the default
/// PrimeGenerator::fill() algorithm which uses __builtin_ctzll().
///
void PrimeGenerator::fill(std::vector<uint64_t>& primes,
                          size_t* size)
{
  do
  {
    if (sieveIdx_ >= sieveSize_)
      if (!sieveSegment(primes, size))
        return;

    *size = 0;
    uint64_t maxSize = primes.size();
    assert(primes.size() >= 64);

    __m512i avxBitValues = _mm512_set_epi8(
      (char) 241, (char) 239, (char) 233, (char) 229,
      (char) 227, (char) 223, (char) 221, (char) 217,
      (char) 211, (char) 209, (char) 203, (char) 199,
      (char) 197, (char) 193, (char) 191, (char) 187,
      (char) 181, (char) 179, (char) 173, (char) 169,
      (char) 167, (char) 163, (char) 161, (char) 157,
      (char) 151, (char) 149, (char) 143, (char) 139,
      (char) 137, (char) 133, (char) 131, (char) 127,
      (char) 121, (char) 119, (char) 113, (char) 109,
      (char) 107, (char) 103, (char) 101, (char)  97,
      (char)  91, (char)  89, (char)  83, (char)  79,
      (char)  77, (char)  73, (char)  71, (char)  67,
      (char)  61, (char)  59, (char)  53, (char)  49,
      (char)  47, (char)  43, (char)  41, (char)  37,
      (char)  31, (char)  29, (char)  23, (char)  19,
      (char)  17, (char)  13, (char)  11, (char)   7
    );

    __m512i bytes_0_to_7   = _mm512_setr_epi64( 0,  1,  2,  3,  4,  5,  6,  7);
    __m512i bytes_8_to_15  = _mm512_setr_epi64( 8,  9, 10, 11, 12, 13, 14, 15);
    __m512i bytes_16_to_23 = _mm512_setr_epi64(16, 17, 18, 19, 20, 21, 22, 23);
    __m512i bytes_24_to_31 = _mm512_setr_epi64(24, 25, 26, 27, 28, 29, 30, 31);
    __m512i bytes_32_to_39 = _mm512_setr_epi64(32, 33, 34, 35, 36, 37, 38, 39);
    __m512i bytes_40_to_47 = _mm512_setr_epi64(40, 41, 42, 43, 44, 45, 46, 47);
    __m512i bytes_48_to_55 = _mm512_setr_epi64(48, 49, 50, 51, 52, 53, 54, 55);
    __m512i bytes_56_to_63 = _mm512_setr_epi64(56, 57, 58, 59, 60, 61, 62, 63);

    while (sieveIdx_ < sieveSize_)
    {
      // Each iteration processes 8 bytes from the sieve array
      uint64_t bits64 = *(uint64_t*) &sieve_[sieveIdx_];
      uint64_t primeCount = popcnt64(bits64);

      // Prevent _mm512_storeu_si512() buffer overrun
      if (*size + primeCount + (8 - primeCount % 8) >= maxSize)
        break;

      __m512i base = _mm512_set1_epi64(low_);
      uint64_t* primes64 = &primes[*size];

      // These variables are not used anymore during this
      // iteration, increment for next iteration.
      *size += primeCount;
      low_ += 8 * 30;
      sieveIdx_ += 8;

      // Convert 1-bits to 0xff bytes
      __m512i bytes64 = _mm512_maskz_set1_epi8(bits64, (char) 0xff);

      // Convert 0xff bytes into prime number offsets
      // using the avxBitValues lookup table.
      __m512i primeOffsets = _mm512_and_si512(bytes64, avxBitValues);

      // Move all non zero bytes (prime offsets) to the beginning
      primeOffsets = _mm512_maskz_compress_epi8(bits64, primeOffsets);

      // Convert the first 8 bytes (prime offsets)
      // into eight 64-bit prime numbers.
      __m512i vprimes0 = _mm512_maskz_permutexvar_epi8(0x0101010101010101ull, bytes_0_to_7, primeOffsets);
      vprimes0 = _mm512_add_epi64(base, vprimes0);
      _mm512_storeu_si512(&primes64[0], vprimes0);

      if (primeCount <= 8)
        continue;

      __m512i vprimes1 = _mm512_maskz_permutexvar_epi8(0x0101010101010101ull, bytes_8_to_15, primeOffsets);
      vprimes1 = _mm512_add_epi64(base, vprimes1);
      _mm512_storeu_si512(&primes64[8], vprimes1);

      if (primeCount <= 16)
        continue;

      __m512i vprimes2 = _mm512_maskz_permutexvar_epi8(0x0101010101010101ull, bytes_16_to_23, primeOffsets);
      vprimes2 = _mm512_add_epi64(base, vprimes2);
      _mm512_storeu_si512(&primes64[16], vprimes2);

      if (primeCount <= 24)
        continue;

      __m512i vprimes3 = _mm512_maskz_permutexvar_epi8(0x0101010101010101ull, bytes_24_to_31, primeOffsets);
      vprimes3 = _mm512_add_epi64(base, vprimes3);
      _mm512_storeu_si512(&primes64[24], vprimes3);

      if (primeCount <= 32)
        continue;

      __m512i vprimes4 = _mm512_maskz_permutexvar_epi8(0x0101010101010101ull, bytes_32_to_39, primeOffsets);
      vprimes4 = _mm512_add_epi64(base, vprimes4);
      _mm512_storeu_si512(&primes64[32], vprimes4);

      if (primeCount <= 40)
        continue;

      __m512i vprimes5 = _mm512_maskz_permutexvar_epi8(0x0101010101010101ull, bytes_40_to_47, primeOffsets);
      vprimes5 = _mm512_add_epi64(base, vprimes5);
      _mm512_storeu_si512(&primes64[40], vprimes5);

      if (primeCount <= 48)
        continue;

      __m512i vprimes6 = _mm512_maskz_permutexvar_epi8(0x0101010101010101ull, bytes_48_to_55, primeOffsets);
      vprimes6 = _mm512_add_epi64(base, vprimes6);
      _mm512_storeu_si512(&primes64[48], vprimes6);

      if (primeCount <= 56)
        continue;

      __m512i vprimes7 = _mm512_maskz_permutexvar_epi8(0x0101010101010101ull, bytes_56_to_63, primeOffsets);
      vprimes7 = _mm512_add_epi64(base, vprimes7);
      _mm512_storeu_si512(&primes64[56], vprimes7);
    }
  }
  while (*size == 0);
}

#else

/// This method is used by iterator::next_prime().
/// This method stores only the next few primes (~ 200) in the
/// primes vector. Also for iterator::next_prime() there is no
/// recurring initialization overhead (unlike prev_prime()) for
/// this reason iterator::next_prime() runs up to 2x faster
/// than iterator::prev_prime().
///
void PrimeGenerator::fill(std::vector<uint64_t>& primes,
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
      i += popcnt64(bits);

      do
      {
        assert(j + 4 < maxSize);
        primes[j+0] = nextPrime(bits, low); bits &= bits - 1;
        primes[j+1] = nextPrime(bits, low); bits &= bits - 1;
        primes[j+2] = nextPrime(bits, low); bits &= bits - 1;
        primes[j+3] = nextPrime(bits, low); bits &= bits - 1;
        j += 4;
      }
      while (j < i);

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

#endif

} // namespace
