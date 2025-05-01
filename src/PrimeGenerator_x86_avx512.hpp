///
/// @file PrimeGenerator_x86_avx512.hpp
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRIMEGENERATOR_X86_AVX512_HPP
#define PRIMEGENERATOR_X86_AVX512_HPP

#include "PrimeGenerator.hpp"

#include <primesieve/macros.hpp>
#include <primesieve/popcnt.hpp>
#include <primesieve/Vector.hpp>

#include <stdint.h>
#include <immintrin.h>
#include <cstddef>

namespace primesieve {

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
/// benchmarks this algorithm ran about 10% faster than the default
/// fillNextPrimes() algorithm which uses __builtin_ctzll().
///
#if defined(ENABLE_MULTIARCH_AVX512_VBMI2)
  __attribute__ ((target ("avx512f,avx512vbmi,avx512vbmi2")))
#endif
void PrimeGenerator::fillNextPrimes_x86_avx512(Vector<uint64_t>& primes, std::size_t* size)
{
  *size = 0;

  do
  {
    if (sieveIdx_ >= sieve_.size())
      if (!sieveNextPrimes(primes, size))
        return;

    // Use local variables to prevent the compiler from
    // writing temporary results to memory.
    std::size_t i = *size;
    std::size_t maxSize = primes.size();
    ASSERT(i + 64 <= maxSize);
    uint64_t low = low_;
    uint64_t sieveIdx = sieveIdx_;
    uint64_t sieveSize = sieve_.size();
    uint8_t* sieve = sieve_.data();

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

    while (sieveIdx < sieveSize)
    {
      // Each iteration processes 8 bytes from the sieve array
      uint64_t bits64 = *(uint64_t*) &sieve[sieveIdx];
      uint64_t primeCount = popcnt64(bits64);

      // Prevent _mm512_storeu_si512() buffer overrun
      if (i + primeCount > maxSize - 8)
        break;

      __m512i base = _mm512_set1_epi64(low);
      uint64_t* primes64 = &primes[i];

      // These variables are not used anymore during this
      // iteration, increment for next iteration.
      i += primeCount;
      low += 8 * 30;
      sieveIdx += 8;

      // Convert 1 bits from the sieve array (bits64) into prime
      // bit values (bytes) using the avxBitValues lookup table and
      // move all non zero bytes (bit values) to the beginning.
      __m512i bitValues = _mm512_maskz_compress_epi8(bits64, avxBitValues);

      // Convert the first 8 bytes (prime bit values)
      // into eight 64-bit prime numbers.
      __m512i vprimes0 = _mm512_maskz_permutexvar_epi8(0x0101010101010101ull, bytes_0_to_7, bitValues);
      vprimes0 = _mm512_add_epi64(base, vprimes0);
      _mm512_storeu_si512(&primes64[0], vprimes0);

      if (primeCount <= 8)
        continue;

      __m512i vprimes1 = _mm512_maskz_permutexvar_epi8(0x0101010101010101ull, bytes_8_to_15, bitValues);
      vprimes1 = _mm512_add_epi64(base, vprimes1);
      _mm512_storeu_si512(&primes64[8], vprimes1);

      if (primeCount <= 16)
        continue;

      __m512i vprimes2 = _mm512_maskz_permutexvar_epi8(0x0101010101010101ull, bytes_16_to_23, bitValues);
      vprimes2 = _mm512_add_epi64(base, vprimes2);
      _mm512_storeu_si512(&primes64[16], vprimes2);

      if (primeCount <= 24)
        continue;

      __m512i vprimes3 = _mm512_maskz_permutexvar_epi8(0x0101010101010101ull, bytes_24_to_31, bitValues);
      vprimes3 = _mm512_add_epi64(base, vprimes3);
      _mm512_storeu_si512(&primes64[24], vprimes3);

      if (primeCount <= 32)
        continue;

      __m512i vprimes4 = _mm512_maskz_permutexvar_epi8(0x0101010101010101ull, bytes_32_to_39, bitValues);
      vprimes4 = _mm512_add_epi64(base, vprimes4);
      _mm512_storeu_si512(&primes64[32], vprimes4);

      if (primeCount <= 40)
        continue;

      __m512i vprimes5 = _mm512_maskz_permutexvar_epi8(0x0101010101010101ull, bytes_40_to_47, bitValues);
      vprimes5 = _mm512_add_epi64(base, vprimes5);
      _mm512_storeu_si512(&primes64[40], vprimes5);

      if (primeCount <= 48)
        continue;

      __m512i vprimes6 = _mm512_maskz_permutexvar_epi8(0x0101010101010101ull, bytes_48_to_55, bitValues);
      vprimes6 = _mm512_add_epi64(base, vprimes6);
      _mm512_storeu_si512(&primes64[48], vprimes6);

      if (primeCount <= 56)
        continue;

      __m512i vprimes7 = _mm512_maskz_permutexvar_epi8(0x0101010101010101ull, bytes_56_to_63, bitValues);
      vprimes7 = _mm512_add_epi64(base, vprimes7);
      _mm512_storeu_si512(&primes64[56], vprimes7);
    }

    low_ = low;
    sieveIdx_ = sieveIdx;
    *size = i;
  }
  while (*size == 0);
}

/// This method is used by iterator::prev_prime().
/// This method stores all primes inside [a, b] into the primes
/// vector. (b - a) is about sqrt(stop) so the memory usage is
/// quite large. Also after primesieve::iterator has iterated
/// over the primes inside [a, b] we need to generate new
/// primes which incurs an initialization overhead of O(sqrt(n)).
///
#if defined(ENABLE_MULTIARCH_AVX512_VBMI2)
  __attribute__ ((target ("avx512f,avx512vbmi,avx512vbmi2")))
#endif
void PrimeGenerator::fillPrevPrimes_x86_avx512(Vector<uint64_t>& primes, std::size_t* size)
{
  *size = 0;

  while (sievePrevPrimes(primes, size))
  {
    // Use local variables to prevent the compiler from
    // writing temporary results to memory.
    std::size_t i = *size;
    uint64_t low = low_;
    uint64_t sieveIdx = sieveIdx_;
    uint64_t sieveSize = sieve_.size();
    uint8_t* sieve = sieve_.data();

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

    while (sieveIdx < sieveSize)
    {
      // Each iteration processes 8 bytes from the sieve array
      uint64_t bits64 = *(uint64_t*) &sieve[sieveIdx];
      uint64_t primeCount = popcnt64(bits64);

      // Prevent _mm512_storeu_si512() buffer overrun
      if_unlikely(i + primeCount + 8 > primes.size())
        primes.resize(i + primeCount + 8);

      __m512i base = _mm512_set1_epi64(low);
      uint64_t* primes64 = &primes[i];

      // These variables are not used anymore during this
      // iteration, increment for next iteration.
      i += primeCount;
      low += 8 * 30;
      sieveIdx += 8;

      // Convert 1 bits from the sieve array (bits64) into prime
      // bit values (bytes) using the avxBitValues lookup table and
      // move all non zero bytes (bit values) to the beginning.
      __m512i bitValues = _mm512_maskz_compress_epi8(bits64, avxBitValues);

      // Convert the first 8 bytes (prime bit values)
      // into eight 64-bit prime numbers.
      __m512i vprimes0 = _mm512_maskz_permutexvar_epi8(0x0101010101010101ull, bytes_0_to_7, bitValues);
      vprimes0 = _mm512_add_epi64(base, vprimes0);
      _mm512_storeu_si512(&primes64[0], vprimes0);

      if (primeCount <= 8)
        continue;

      __m512i vprimes1 = _mm512_maskz_permutexvar_epi8(0x0101010101010101ull, bytes_8_to_15, bitValues);
      vprimes1 = _mm512_add_epi64(base, vprimes1);
      _mm512_storeu_si512(&primes64[8], vprimes1);

      if (primeCount <= 16)
        continue;

      __m512i vprimes2 = _mm512_maskz_permutexvar_epi8(0x0101010101010101ull, bytes_16_to_23, bitValues);
      vprimes2 = _mm512_add_epi64(base, vprimes2);
      _mm512_storeu_si512(&primes64[16], vprimes2);

      if (primeCount <= 24)
        continue;

      __m512i vprimes3 = _mm512_maskz_permutexvar_epi8(0x0101010101010101ull, bytes_24_to_31, bitValues);
      vprimes3 = _mm512_add_epi64(base, vprimes3);
      _mm512_storeu_si512(&primes64[24], vprimes3);

      if (primeCount <= 32)
        continue;

      __m512i vprimes4 = _mm512_maskz_permutexvar_epi8(0x0101010101010101ull, bytes_32_to_39, bitValues);
      vprimes4 = _mm512_add_epi64(base, vprimes4);
      _mm512_storeu_si512(&primes64[32], vprimes4);

      if (primeCount <= 40)
        continue;

      __m512i vprimes5 = _mm512_maskz_permutexvar_epi8(0x0101010101010101ull, bytes_40_to_47, bitValues);
      vprimes5 = _mm512_add_epi64(base, vprimes5);
      _mm512_storeu_si512(&primes64[40], vprimes5);

      if (primeCount <= 48)
        continue;

      __m512i vprimes6 = _mm512_maskz_permutexvar_epi8(0x0101010101010101ull, bytes_48_to_55, bitValues);
      vprimes6 = _mm512_add_epi64(base, vprimes6);
      _mm512_storeu_si512(&primes64[48], vprimes6);

      if (primeCount <= 56)
        continue;

      __m512i vprimes7 = _mm512_maskz_permutexvar_epi8(0x0101010101010101ull, bytes_56_to_63, bitValues);
      vprimes7 = _mm512_add_epi64(base, vprimes7);
      _mm512_storeu_si512(&primes64[56], vprimes7);
    }

    low_ = low;
    sieveIdx_ = sieveIdx;
    *size = i;
  }
}

} // namespace

#endif
