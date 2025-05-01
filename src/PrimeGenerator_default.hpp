///
/// @file PrimeGenerator_default.hpp
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
/// Copyright (C) 2022 @zielaj, https://github.com/zielaj
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRIMEGENERATOR_DEFAULT_HPP
#define PRIMEGENERATOR_DEFAULT_HPP

#include "PrimeGenerator.hpp"
#include "Erat.hpp"

#include <primesieve/littleendian_cast.hpp>
#include <primesieve/macros.hpp>
#include <primesieve/popcnt.hpp>
#include <primesieve/Vector.hpp>

#include <stdint.h>
#include <cstddef>

namespace primesieve {

/// This method is used by iterator::next_prime().
/// This method stores only the next few primes (~ 1000) in the
/// primes vector. Also for iterator::next_prime() there is no
/// recurring initialization overhead (unlike prev_prime()) for
/// this reason iterator::next_prime() runs up to 2x faster
/// than iterator::prev_prime().
///
void PrimeGenerator::fillNextPrimes_default(Vector<uint64_t>& primes, std::size_t* size)
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

    // Fill the buffer with at least (maxSize - 64) primes.
    // Each loop iteration can generate up to 64 primes
    // so we have to stop generating primes once there is
    // not enough space for 64 more primes.
    do
    {
      uint64_t bits = littleendian_cast<uint64_t>(&sieve[sieveIdx]);
      std::size_t j = i;
      i += popcnt64(bits);

      do
      {
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

/// This method is used by iterator::prev_prime().
/// This method stores all primes inside [a, b] into the primes
/// vector. (b - a) is about sqrt(stop) so the memory usage is
/// quite large. Also after primesieve::iterator has iterated
/// over the primes inside [a, b] we need to generate new
/// primes which incurs an initialization overhead of O(sqrt(n)).
///
void PrimeGenerator::fillPrevPrimes_default(Vector<uint64_t>& primes, std::size_t* size)
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

    while (sieveIdx < sieveSize)
    {
      // Each loop iteration can generate up to 64 primes,
      // so we have to make sure there is enough space
      // left in the primes vector.
      if_unlikely(i + 64 > primes.size())
        primes.resize(i + 64);

      uint64_t bits = littleendian_cast<uint64_t>(&sieve[sieveIdx]);
      std::size_t j = i;
      i += popcnt64(bits);

      do
      {
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

    low_ = low;
    sieveIdx_ = sieveIdx;
    *size = i;
  }
}

} // namespace

#endif
