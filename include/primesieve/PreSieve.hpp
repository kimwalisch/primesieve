///
/// @file   PreSieve.hpp
/// @brief  Pre-sieve multiples of small primes < 100 to speed up the
///         sieve of Eratosthenes. The idea is to allocate several
///         arrays (buffers_) and remove the multiples of small primes
///         from them at initialization. Each buffer is assigned
///         different primes, for example:
///
///         buffer[0] removes multiplies of: {  7, 67, 71 } // 32 KiB
///         buffer[1] removes multiplies of: { 11, 41, 73 } // 32 KiB
///         buffer[2] removes multiplies of: { 13, 43, 59 } // 32 KiB
///         buffer[3] removes multiplies of: { 17, 37, 53 } // 32 KiB
///         buffer[4] removes multiplies of: { 19, 29, 61 } // 32 KiB
///         buffer[5] removes multiplies of: { 23, 31, 47 } // 32 KiB
///         buffer[6] removes multiplies of: { 79, 97 }     // 30 KiB
///         buffer[7] removes multiplies of: { 83, 89 }     // 29 KiB
///
///         Then whilst sieving, we perform a bitwise AND on the
///         buffers_ arrays and store the result in the sieve array.
///         Pre-sieving provides a speedup of up to 30% when
///         sieving the primes < 10^10 using primesieve.
///
/// Copyright (C) 2024 Kim Walisch, <kim.walisch@gmail.com>
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
  static void preSieve(Vector<uint8_t>& sieve, uint64_t segmentLow);
  static uint64_t getMaxPrime() { return 97; }
};

} // namespace

#endif
