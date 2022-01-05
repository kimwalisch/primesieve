///
/// @file  PreSieve.hpp
///
/// Copyright (C) 2020 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRESIEVE_HPP
#define PRESIEVE_HPP

#include <stdint.h>
#include <vector>
#include <array>

namespace primesieve {

/// PreSieve objects are used to pre-sieve multiples of small primes
/// e.g. <= 19 to speed up the sieve of Eratosthenes. The idea is to
/// allocate several arrays (buffers_) and remove the multiples of
/// small primes from them at initialization. Each buffer is assigned
/// different primes, for example:
///
/// Buffer 0 crosses off multiplies of 7, 19, 23, 29,    uses  89 kB
/// Buffer 1 crosses off multiplies of 11, 13, 17, 37    uses  90 kB
/// Buffer 2 crosses off multiplies of 31, 47, 59        uses  86 kB
/// Buffer 3 crosses off multiplies of 41, 43, 53        uses  93 kB
///
/// Then whilst sieving, we perform a bitwise AND on the the buffers_
/// arrays, and stare the result in the sieve array at the beginning
/// of each new segment to pre-sieve the multiples of small primes <=
/// maxPrime_. Pre-sieving with a single buffer speeds up my sieve of
/// Eratosthenes implementation by about 20 percent when sieving <
/// 10^10. Pre-sieving with multiple buffers provides an additional
/// 5-8% speed up.

class PreSieve
{
public:
  void init(uint64_t, uint64_t);
  uint64_t getMaxPrime() const { return maxPrime_; }
  void copy(uint8_t*, uint64_t, uint64_t) const;
private:
  uint64_t maxPrime_ = 0;

  static constexpr int BUFFERS = 4;
  std::array<std::vector<uint8_t>, BUFFERS> buffers_;

  void initBuffer(uint64_t, uint64_t);
};

} // namespace

#endif
