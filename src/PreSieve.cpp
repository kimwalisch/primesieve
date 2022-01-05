///
/// @file   PreSieve.cpp
/// @brief  Pre-sieve multiples of small primes to speed up the sieve
///         of Eratosthenes. At startup primesieve initializes a small
///         buffer of size p1*p2*p3*pn and removes the multiples of
///         the first n primes from that buffer. Then while sieving at
///         the start of each new segment this buffer is simply copied
///         to the sieve array and now we can start sieving at p(n+1)
///         instead of p1. By default primesieve pre-sieves multiples
///         of primes <= 19, in practice pre-sieving using even larger
///         primes uses too much memory and slows things down. In
///         primesieve pre-sieving provides a minor speed of up to 20%
///         when the sieving distance is relatively small
///         e.g. < 10^10.
///
///         The pre-sieve buffer can be both smaller or larger than
///         the actual sieve array so a little care needs to be taken
///         when copying the buffer to the sieve array. When the
///         buffer is smaller than the sieve array we need to
///         repeatedly copy the buffer to the sieve array until the
///         sieve array has been filled completely. When the buffer is
///         larger than the sieve array we only need to partially copy
///         it to the sieve array.
///
///         TODO: update this description
///
/// Copyright (C) 2020 Kim Walisch, <kim.walisch@gmail.com>
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
#include <numeric>

using namespace std;

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

  if (maxPrime_ != 0) return;  // Already initialized.

  static const std::vector<std::vector<uint64_t>> bufferPrimes =
    // {{7, 41, 43},   // 12 kB
    //  {11, 23, 47},  // 12 kB
    //  {13, 29, 31},  // 14 kB
    //  {17, 19, 37}}; // 12 kB

    {{7, 19, 23, 29},  // 89 kB
     {11, 13, 17, 37}, // 90 kB
     {31, 47, 59},     // 86 kB
     {41, 43, 53}};    // 93 kB

    // {{7, 41, 47, 53},    // 715 kB
    //  {11, 29, 37, 61},   // 720 kB
    //  {13, 19, 43, 67},   // 712 kB
    //  {17, 23, 31, 59}};  // 715 kB

  for (int i = 0; i < BUFFERS; ++i) {
    uint64_t product = 30 * std::accumulate(bufferPrimes[i].begin(), bufferPrimes[i].end(), 1,
                                            std::multiplies());
    uint64_t maxPrime = *std::max_element(bufferPrimes[i].begin(), bufferPrimes[i].end());
    maxPrime_ = max(maxPrime_, maxPrime);

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

/// Perform a per-element bitwise AND operation on buf1..4, and store the result in output.
void AndBuffers(const uint8_t* buf1, const uint8_t* buf2,
                const uint8_t* buf3, const uint8_t* buf4,
                uint8_t* output, int bytes) {
  // This loop should be auto-vectorized.
  for (int i = 0; i < bytes; ++i)
    output[i] = buf1[i] & buf2[i] & buf3[i] & buf4[i];
}

/// Populate `sieve` based on internal pre-sieved buffers.
void PreSieve::copy(uint8_t* sieve,
                    uint64_t sieveSize,
                    uint64_t segmentLow) const
{
  std::array<uint64_t, BUFFERS> pos;
  for (int i = 0; i < BUFFERS; ++i)
    pos[i] = (segmentLow % (buffers_[i].size() * 30)) / 30;

  uint64_t offset = 0;
  while (offset < sieveSize) {
    uint64_t bytesToCopy = sieveSize - offset;
    for (int i = 0; i < BUFFERS; ++i) {
      uint64_t left = buffers_[i].size() - pos[i];
      if (left < bytesToCopy) bytesToCopy = left;
    }

    AndBuffers(buffers_[0].data() + pos[0],
               buffers_[1].data() + pos[1],
               buffers_[2].data() + pos[2],
               buffers_[3].data() + pos[3],
               sieve + offset,
               bytesToCopy);

    offset += bytesToCopy;
    for (int i = 0; i < BUFFERS; ++i) {
      pos[i] += bytesToCopy;
      if (pos[i] >= buffers_[i].size()) pos[i] = 0;
    }
  }
}

} // namespace
