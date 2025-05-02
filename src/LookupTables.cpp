///
/// @file   LookupTables.cpp
/// @brief  Static gobal arrays.
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include "Wheel.hpp"

#include <primesieve/forward.hpp>
#include <primesieve/Vector.hpp>

#include <stdint.h>

namespace primesieve {

/// primesieve uses a bit array for sieving in which the 8 bits
/// of each byte correspond to the offsets:
/// { 1, 7, 11, 13, 17, 19, 23, 29 }. However, in order to more
/// efficiently sieve prime k-tuplets (e.g. twin primes) we
/// rearrange these offsets to { 7, 11, 13, 17, 19, 23, 29, 31 }.
/// 64 bits of the sieve array correspond to 8 bytes which span
/// an interval of size 30 * 8 = 240.
///
/// The index for this lookup table is computed using the count
/// trailing zeros CPU instruction. As a special case CTZ may return
/// the operand size (number of bits) if the input is zero. Hence
/// the maximum index is 64 for e.g. TZCNT(0) (on x64 CPUs)
/// therefore we add an additional 0 at the end of the array to
/// prevent out of bounds acceses.
///
const Array<uint64_t, 65> bitValues =
{
    7,  11,  13,  17,  19,  23,  29, 31,
   37,  41,  43,  47,  49,  53,  59, 61,
   67,  71,  73,  77,  79,  83,  89, 91,
   97, 101, 103, 107, 109, 113, 119, 121,
  127, 131, 133, 137, 139, 143, 149, 151,
  157, 161, 163, 167, 169, 173, 179, 181,
  187, 191, 193, 197, 199, 203, 209, 211,
  217, 221, 223, 227, 229, 233, 239, 241,
  0
};

/// The De Bruijn bitscan is a fast method to compute the index of
/// the first set bit in a 64-bit integer using only integer
/// operations. For primesieve's use case this is as fast as the
/// bsf or tzcnt instructions on x64 (but more portable).
/// https://www.chessprogramming.org/BitScan#De_Bruijn_Multiplication
///
const Array<uint64_t, 64> bruijnBitValues =
{
    7,  47,  11,  49,  67, 113,  13,  53,
   89,  71, 161, 101, 119, 187,  17, 233,
   59,  79,  91,  73, 133, 139, 163, 103,
  149, 121, 203, 169, 191, 217,  19, 239,
   43,  61, 109,  83, 157,  97, 181, 229,
   77, 131, 137, 143, 199, 167, 211,  41,
  107, 151, 179, 227, 127, 197, 209,  37,
  173, 223, 193,  31, 221,  29,  23, 241
};

/// Used to find the next multiple (of a prime)
/// that is not divisible by 2, 3 and 5.
///
const WheelInit wheel30Init[30] =
{
  { 1, 0 }, { 0, 0 }, { 5, 1 }, { 4, 1 }, { 3, 1}, { 2, 1 }, { 1, 1 }, { 0, 1 },
  { 3, 2 }, { 2, 2 }, { 1, 2 }, { 0, 2 }, { 1, 3}, { 0, 3 }, { 3, 4 }, { 2, 4 },
  { 1, 4 }, { 0, 4 }, { 1, 5 }, { 0, 5 }, { 3, 6}, { 2, 6 }, { 1, 6 }, { 0, 6 },
  { 5, 7 }, { 4, 7 }, { 3, 7 }, { 2, 7 }, { 1, 7}, { 0, 7 }
};

/// Used to find the next multiple (of a prime)
/// that is not divisible by 2, 3, 5 and 7.
///
const WheelInit wheel210Init[210] =
{
  { 1, 0 },  { 0, 0 },  { 9, 1 },  { 8, 1 },  { 7, 1 },  { 6, 1 },  { 5, 1 },  { 4, 1 },
  { 3, 1 },  { 2, 1 },  { 1, 1 },  { 0, 1 },  { 1, 2 },  { 0, 2 },  { 3, 3 },  { 2, 3 },
  { 1, 3 },  { 0, 3 },  { 1, 4 },  { 0, 4 },  { 3, 5 },  { 2, 5 },  { 1, 5 },  { 0, 5 },
  { 5, 6 },  { 4, 6 },  { 3, 6 },  { 2, 6 },  { 1, 6 },  { 0, 6 },  { 1, 7 },  { 0, 7 },
  { 5, 8 },  { 4, 8 },  { 3, 8 },  { 2, 8 },  { 1, 8 },  { 0, 8 },  { 3, 9 },  { 2, 9 },
  { 1, 9 },  { 0, 9 },  { 1, 10 }, { 0, 10 }, { 3, 11 }, { 2, 11 }, { 1, 11 }, { 0, 11 },
  { 5, 12 }, { 4, 12 }, { 3, 12 }, { 2, 12 }, { 1, 12 }, { 0, 12 }, { 5, 13 }, { 4, 13 },
  { 3, 13 }, { 2, 13 }, { 1, 13 }, { 0, 13 }, { 1, 14 }, { 0, 14 }, { 5, 15 }, { 4, 15 },
  { 3, 15 }, { 2, 15 }, { 1, 15 }, { 0, 15 }, { 3, 16 }, { 2, 16 }, { 1, 16 }, { 0, 16 },
  { 1, 17 }, { 0, 17 }, { 5, 18 }, { 4, 18 }, { 3, 18 }, { 2, 18 }, { 1, 18 }, { 0, 18 },
  { 3, 19 }, { 2, 19 }, { 1, 19 }, { 0, 19 }, { 5, 20 }, { 4, 20 }, { 3, 20 }, { 2, 20 },
  { 1, 20 }, { 0, 20 }, { 7, 21 }, { 6, 21 }, { 5, 21 }, { 4, 21 }, { 3, 21 }, { 2, 21 },
  { 1, 21 }, { 0, 21 }, { 3, 22 }, { 2, 22 }, { 1, 22 }, { 0, 22 }, { 1, 23 }, { 0, 23 },
  { 3, 24 }, { 2, 24 }, { 1, 24 }, { 0, 24 }, { 1, 25 }, { 0, 25 }, { 3, 26 }, { 2, 26 },
  { 1, 26 }, { 0, 26 }, { 7, 27 }, { 6, 27 }, { 5, 27 }, { 4, 27 }, { 3, 27 }, { 2, 27 },
  { 1, 27 }, { 0, 27 }, { 5, 28 }, { 4, 28 }, { 3, 28 }, { 2, 28 }, { 1, 28 }, { 0, 28 },
  { 3, 29 }, { 2, 29 }, { 1, 29 }, { 0, 29 }, { 5, 30 }, { 4, 30 }, { 3, 30 }, { 2, 30 },
  { 1, 30 }, { 0, 30 }, { 1, 31 }, { 0, 31 }, { 3, 32 }, { 2, 32 }, { 1, 32 }, { 0, 32 },
  { 5, 33 }, { 4, 33 }, { 3, 33 }, { 2, 33 }, { 1, 33 }, { 0, 33 }, { 1, 34 }, { 0, 34 },
  { 5, 35 }, { 4, 35 }, { 3, 35 }, { 2, 35 }, { 1, 35 }, { 0, 35 }, { 5, 36 }, { 4, 36 },
  { 3, 36 }, { 2, 36 }, { 1, 36 }, { 0, 36 }, { 3, 37 }, { 2, 37 }, { 1, 37 }, { 0, 37 },
  { 1, 38 }, { 0, 38 }, { 3, 39 }, { 2, 39 }, { 1, 39 }, { 0, 39 }, { 5, 40 }, { 4, 40 },
  { 3, 40 }, { 2, 40 }, { 1, 40 }, { 0, 40 }, { 1, 41 }, { 0, 41 }, { 5, 42 }, { 4, 42 },
  { 3, 42 }, { 2, 42 }, { 1, 42 }, { 0, 42 }, { 3, 43 }, { 2, 43 }, { 1, 43 }, { 0, 43 },
  { 1, 44 }, { 0, 44 }, { 3, 45 }, { 2, 45 }, { 1, 45 }, { 0, 45 }, { 1, 46 }, { 0, 46 },
  { 9, 47 }, { 8, 47 }, { 7, 47 }, { 6, 47 }, { 5, 47 }, { 4, 47 }, { 3, 47 }, { 2, 47 },
  { 1, 47 }, { 0, 47 }
};

} // namespace
