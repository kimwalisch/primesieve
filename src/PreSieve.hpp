///
/// @file   PreSieve.hpp
/// @brief  Pre-sieve multiples of small primes <= 163 to speed up the
///         sieve of Eratosthenes. We use 16 static lookup tables from
///         which the multiples of small primes have been removed
///         upfront. Each preSieve lookup table is assigned different
///         primes used for pre-sieving:
///
///         preSieveTable[0]  = {  7, 23, 37 }
///         preSieveTable[1]  = { 11, 19, 31 }
///         preSieveTable[2]  = { 13, 17, 29 }
///         preSieveTable[3]  = { 41, 163 }
///         preSieveTable[4]  = { 43, 157 }
///         preSieveTable[5]  = { 47, 151 }
///         preSieveTable[6]  = { 53, 149 }
///         preSieveTable[7]  = { 59, 139 }
///         preSieveTable[8]  = { 61, 137 }
///         preSieveTable[9]  = { 67, 131 }
///         preSieveTable[10] = { 71, 127 }
///         preSieveTable[11] = { 73, 113 }
///         preSieveTable[12] = { 79, 109 }
///         preSieveTable[13] = { 83, 107 }
///         preSieveTable[14] = { 89, 103 }
///         preSieveTable[15] = { 97, 101 }
///
///         The total size of these 16 preSieveTables is 123
///         kilobytes. Whilst sieving, we perform a bitwise AND of all
///         preSieveTables and store the result in the sieve array.
///         Pre-sieving provides a speedup of up to 30% when sieving
///         the primes < 10^10 using primesieve.
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
/// Copyright (C) 2022 @zielaj, https://github.com/zielaj
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRESIEVE_HPP
#define PRESIEVE_HPP

#include <primesieve/Vector.hpp>
#include <stdint.h>

namespace primesieve {

class PreSieve
{
public:
  static void preSieve(Vector<uint8_t>& sieve, uint64_t segmentLow);
  static uint64_t getMaxPrime() { return 163; }
};

} // namespace

#endif
