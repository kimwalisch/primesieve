///
/// @file   PreSieve.hpp
/// @brief  Pre-sieve multiples of small primes < 100 to speed up the
///         sieve of Eratosthenes. The idea is to allocate several
///         arrays (buffers_) and remove the multiples of small primes
///         from them at initialization. Each buffer is assigned
///         different primes, for example:
///
///         Buffer 0 removes multiplies of: {  7, 67, 71 }
///         Buffer 1 removes multiplies of: { 11, 41, 73 }
///         Buffer 2 removes multiplies of: { 13, 43, 59 }
///         Buffer 3 removes multiplies of: { 17, 37, 53 }
///         Buffer 4 removes multiplies of: { 19, 29, 61 }
///         Buffer 5 removes multiplies of: { 23, 31, 47 }
///         Buffer 6 removes multiplies of: { 79, 97 }
///         Buffer 7 removes multiplies of: { 83, 89 }
///
///         Then whilst sieving, we perform a bitwise AND on the
///         buffers_ arrays and store the result in the sieve array.
///         Pre-sieving provides a speedup of up to 30% when
///         sieving the primes < 10^10 using primesieve.
///
/// Copyright (C) 2023 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRESIEVE_HPP
#define PRESIEVE_HPP

#include "Vector.hpp"
#include <stdint.h>

namespace primesieve {

class PreSieve
{
public:
  void init(uint64_t start, uint64_t stop);
  void preSieve(Vector<uint8_t>& sieve, uint64_t segmentLow) const;
  uint64_t getMaxPrime() const { return maxPrime_; }
private:
  uint64_t maxPrime_ = 13;
  uint64_t totalDist_ = 0;
  Array<Vector<uint8_t>, 8> buffers_;
  void initBuffers();
  static void preSieveSmall(Vector<uint8_t>& sieve, uint64_t segmentLow);
  void preSieveLarge(Vector<uint8_t>& sieve, uint64_t segmentLow) const;
};

} // namespace

#endif
