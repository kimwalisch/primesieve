///
/// @file PrimeGenerator_x86_avx512.hpp
///
/// Copyright (C) 2026 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRIMEGENERATOR_X86_AVX512_HPP
#define PRIMEGENERATOR_X86_AVX512_HPP

#include "PrimeGenerator.hpp"

#include <primesieve/macros.hpp>
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
/// For each sieve word we unconditionally store the first 16 primes
/// (unused entries are overwritten by the next sieve word), this
/// avoids hard to predict branches that check the prime count.
///
#if defined(ENABLE_MULTIARCH_AVX512_VBMI2)
  PRIMESIEVE_MULTIARCH_KERNEL("avx512f,avx512vbmi,avx512vbmi2,popcnt")
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
    const uint64_t* sieve = sieve_.data();
    uint64_t* primes64 = primes.data();

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

    __m512i bytes_0_to_7  = _mm512_setr_epi64( 0,  1,  2,  3,  4,  5,  6,  7);
    __m512i bytes_8_to_15 = _mm512_setr_epi64( 8,  9, 10, 11, 12, 13, 14, 15);
    __m512i base = _mm512_set1_epi64(low);
    __m512i baseStep = _mm512_set1_epi64(8 * 30);

    // Prevent _mm512_storeu_si512() buffer overrun,
    // a sieve word generates up to 64 primes.
    while (i <= maxSize - 64 &&
           sieveIdx < sieveSize)
    {
      // Each iteration processes 8 bytes from the sieve array
      uint64_t bits64 = sieve[sieveIdx];
      uint64_t primeCount = _mm_popcnt_u64(bits64);

      // Convert 1 bits from the sieve array (bits64) into prime
      // bit values (bytes) using the avxBitValues lookup table and
      // move all non zero bytes (bit values) to the beginning.
      __m512i bitValues = _mm512_maskz_compress_epi8(bits64, avxBitValues);

      // Convert the first 16 bytes (prime bit values)
      // into sixteen 64-bit prime numbers.
      __m512i vprimes0 = _mm512_maskz_permutexvar_epi8(0x0101010101010101, bytes_0_to_7, bitValues);
      vprimes0 = _mm512_add_epi64(base, vprimes0);
      _mm512_storeu_si512(&primes64[i + 0], vprimes0);

      __m512i vprimes1 = _mm512_maskz_permutexvar_epi8(0x0101010101010101, bytes_8_to_15, bitValues);
      vprimes1 = _mm512_add_epi64(base, vprimes1);
      _mm512_storeu_si512(&primes64[i + 8], vprimes1);

      // Rare case: the sieve word contains more than 16 primes
      if_unlikely(primeCount > 16)
      {
        __m512i bytes = bytes_8_to_15;
        __m512i eight = _mm512_set1_epi64(8);

        NO_UNROLL_LOOP
        for (uint64_t j = 16; j < primeCount; j += 8)
        {
          bytes = _mm512_add_epi64(bytes, eight);
          __m512i vprimes = _mm512_maskz_permutexvar_epi8(0x0101010101010101, bytes, bitValues);
          vprimes = _mm512_add_epi64(base, vprimes);
          _mm512_storeu_si512(&primes64[i + j], vprimes);
        }
      }

      i += primeCount;
      sieveIdx++;
      base = _mm512_add_epi64(base, baseStep);
    }

    low_ = low + (sieveIdx - sieveIdx_) * 240;
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
  PRIMESIEVE_MULTIARCH_KERNEL("avx512f,avx512vbmi,avx512vbmi2,popcnt")
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
    const uint64_t* sieve = sieve_.data();

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

    __m512i bytes_0_to_7  = _mm512_setr_epi64( 0,  1,  2,  3,  4,  5,  6,  7);
    __m512i bytes_8_to_15 = _mm512_setr_epi64( 8,  9, 10, 11, 12, 13, 14, 15);
    __m512i base = _mm512_set1_epi64(low);
    __m512i baseStep = _mm512_set1_epi64(8 * 30);

    while (sieveIdx < sieveSize)
    {
      // Each iteration processes 8 bytes from the sieve array
      uint64_t bits64 = sieve[sieveIdx];
      uint64_t primeCount = _mm_popcnt_u64(bits64);

      // Prevent _mm512_storeu_si512() buffer overrun
      if_unlikely(i + primeCount + 16 > primes.size())
        primes.resize(i + primeCount + 16);

      uint64_t* primes64 = primes.data();

      // Convert 1 bits from the sieve array (bits64) into prime
      // bit values (bytes) using the avxBitValues lookup table and
      // move all non zero bytes (bit values) to the beginning.
      __m512i bitValues = _mm512_maskz_compress_epi8(bits64, avxBitValues);

      // Convert the first 16 bytes (prime bit values)
      // into sixteen 64-bit prime numbers.
      __m512i vprimes0 = _mm512_maskz_permutexvar_epi8(0x0101010101010101, bytes_0_to_7, bitValues);
      vprimes0 = _mm512_add_epi64(base, vprimes0);
      _mm512_storeu_si512(&primes64[i + 0], vprimes0);

      __m512i vprimes1 = _mm512_maskz_permutexvar_epi8(0x0101010101010101, bytes_8_to_15, bitValues);
      vprimes1 = _mm512_add_epi64(base, vprimes1);
      _mm512_storeu_si512(&primes64[i + 8], vprimes1);

      // Rare case: the sieve word contains more than 16 primes
      if_unlikely(primeCount > 16)
      {
        __m512i bytes = bytes_8_to_15;
        __m512i eight = _mm512_set1_epi64(8);

        NO_UNROLL_LOOP
        for (uint64_t j = 16; j < primeCount; j += 8)
        {
          bytes = _mm512_add_epi64(bytes, eight);
          __m512i vprimes = _mm512_maskz_permutexvar_epi8(0x0101010101010101, bytes, bitValues);
          vprimes = _mm512_add_epi64(base, vprimes);
          _mm512_storeu_si512(&primes64[i + j], vprimes);
        }
      }

      i += primeCount;
      sieveIdx++;
      base = _mm512_add_epi64(base, baseStep);
    }

    low_ = low + (sieveIdx - sieveIdx_) * 240;
    sieveIdx_ = sieveIdx;
    *size = i;
  }
}

} // namespace

#endif
