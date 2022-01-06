///
/// @file   PreSieve.cpp
/// @brief  Pre-sieve multiples of small primes <= 59 to speed up the
///         sieve of Eratosthenes. The idea is to allocate several
///         arrays (buffers_) and remove the multiples of small primes
///         from them at initialization. Each buffer is assigned
///         different primes, for example:
///
///         Buffer 0 removes multiplies of:  7, 19, 23, 29
///         Buffer 1 removes multiplies of: 11, 13, 17, 37
///         Buffer 2 removes multiplies of: 31, 47, 59
///         Buffer 3 removes multiplies of: 41, 43, 53
///
///         Then whilst sieving, we perform a bitwise AND on the
///         buffers_ arrays and store the result in the sieve array.
///         Pre-sieving provides a speedup of up to 30% when
///         sieving the primes < 10^10 using primesieve.
///
/// Copyright (C) 2021 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/PreSieve.hpp>
#include <primesieve/EratSmall.hpp>
#include <primesieve/pmath.hpp>

#include <stdint.h>
#include <algorithm>
#include <array>
#include <iterator>
#include <vector>

using std::size_t;

namespace {

/// Pre-sieve with the primes <= 59
const std::array<std::vector<uint64_t>, 4> bufferPrimes =
{{
  {  7, 19, 23, 29 }, // 89 KiB
  { 11, 13, 17, 37 }, // 90 KiB
  { 31, 47, 59 },     // 86 KiB
  { 41, 43, 53 }      // 93 KiB
}};

void andBuffers(const uint8_t* buf1,
                const uint8_t* buf2,
                const uint8_t* buf3,
                const uint8_t* buf4,
                uint8_t* output,
                size_t bytes)
{
  // This loop should be auto-vectorized
  for (size_t i = 0; i < bytes; i++)
    output[i] = buf1[i] & buf2[i] & buf3[i] & buf4[i];
}

} // namespace

namespace primesieve {

void PreSieve::init(uint64_t, uint64_t)
{
  // TODO: resolve this before committing upstream.

  // The original code set up the threshold dynamically, and was able
  // to increase it for the same instance of PreSieve. Implementing
  // this functionality with multiple pre-sieves is more difficult, so
  // let's first understand whether it's still needed.

  // // The pre-sieve buffer should be at least 100
  // // times smaller than the sieving distance
  // // in order to reduce initialization overhead.

  // // uint64_t dist = stop - start;
  // // uint64_t threshold = max(dist, isqrt(stop)) / 100;

  // Already initialized
  if (maxPrime_ != 0)
    return;  

  for (size_t i = 0; i < buffers_.size(); i++) {
    uint64_t product = 30;

    for (auto prime : bufferPrimes[i])
      product *= prime;

    uint64_t maxPrime = bufferPrimes[i].back();
    maxPrime_ = std::max(maxPrime_, maxPrime);

    buffers_[i].clear();
    buffers_[i].resize(product / 30, 0xff);

    EratSmall eratSmall;
    uint64_t start = product;
    assert(start >= maxPrime * maxPrime);
    eratSmall.init(start + product, buffers_[i].size(), maxPrime);

    for (uint64_t prime : bufferPrimes[i])
      eratSmall.addSievingPrime(prime, product);
    eratSmall.crossOff(buffers_[i].data(), buffers_[i].size());
  }
}

/// Populate the sieve array using the buffers that
/// are pre-sieved with the primes <= 59.
///
void PreSieve::copy(uint8_t* sieve,
                    uint64_t sieveSize,
                    uint64_t segmentLow) const
{
  uint64_t offset = 0;
  std::array<uint64_t, bufferPrimes.size()> pos;

  for (size_t i = 0; i < buffers_.size(); i++)
    pos[i] = (segmentLow % (buffers_[i].size() * 30)) / 30;

  while (offset < sieveSize) {
    uint64_t bytesToCopy = sieveSize - offset;

    for (size_t i = 0; i < buffers_.size(); i++) {
      uint64_t left = buffers_[i].size() - pos[i];
      if (left < bytesToCopy)
        bytesToCopy = left;
    }

    andBuffers(&buffers_[0][pos[0]],
               &buffers_[1][pos[1]],
               &buffers_[2][pos[2]],
               &buffers_[3][pos[3]],
               &sieve[offset],
               bytesToCopy);

    offset += bytesToCopy;

    for (size_t i = 0; i < buffers_.size(); i++) {
      pos[i] += bytesToCopy;
      if (pos[i] >= buffers_[i].size())
        pos[i] = 0;
    }
  }
}

} // namespace
