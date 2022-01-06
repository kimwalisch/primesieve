///
/// @file   PreSieve.hpp
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

#ifndef PRESIEVE_HPP
#define PRESIEVE_HPP

#include <stdint.h>
#include <array>
#include <vector>

namespace primesieve {

class PreSieve
{
public:
  void init(uint64_t start, uint64_t stop);
  void preSieve(uint8_t* sieve, uint64_t sieveSize, uint64_t segmentLow) const;
  uint64_t getMaxPrime() const { return maxPrime_; }
private:
  uint64_t maxPrime_ = 13;
  std::array<std::vector<uint8_t>, 4> buffers_;
  void preSieveSmall(uint8_t* sieve, uint64_t sieveSize, uint64_t segmentLow) const;
  void preSieveLarge(uint8_t* sieve, uint64_t sieveSize, uint64_t segmentLow) const;
  static void resetTinyPrimes(uint8_t* sieve, uint64_t segmentLow);
};

} // namespace

#endif
