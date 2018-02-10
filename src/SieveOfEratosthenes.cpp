///
/// @file   SieveOfEratosthenes.cpp
/// @brief  The SieveOfEratosthenes class manages prime sieving
///         using the PreSieve, EratSmall, EratMedium and
///         EratBig classes.
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/config.hpp>
#include <primesieve/SieveOfEratosthenes.hpp>
#include <primesieve/PreSieve.hpp>
#include <primesieve/EratSmall.hpp>
#include <primesieve/EratMedium.hpp>
#include <primesieve/EratBig.hpp>
#include <primesieve/pmath.hpp>
#include <primesieve/primesieve_error.hpp>

#include <stdint.h>
#include <cstring>
#include <memory>

using namespace std;
using namespace primesieve;

namespace {

/// unset bits < start
const byte_t unsetSmaller[32] =
{
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xfe, 0xfe, 0xfe, 0xfe, 0xfc, 0xfc,
  0xf8, 0xf8, 0xf8, 0xf8, 0xf0, 0xf0, 0xe0,
  0xe0, 0xe0, 0xe0, 0xc0, 0xc0, 0xc0, 0xc0,
  0xc0, 0xc0, 0x80, 0x80
};

/// unset bits > stop
const byte_t unsetLarger[32] =
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x01, 0x01, 0x01, 0x01, 0x03, 0x03, 0x07,
  0x07, 0x07, 0x07, 0x0f, 0x0f, 0x1f, 0x1f,
  0x1f, 0x1f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
  0x3f, 0x7f, 0x7f, 0xff
};

} // namespace

namespace primesieve {

/// De Bruijn bitscan table
const uint64_t SieveOfEratosthenes::bruijnBitValues_[64] =
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

/// @start:      Sieve primes >= start
/// @stop:       Sieve primes <= stop
/// @sieveSize:  Sieve size in kilobytes
/// @preSieve:   Pre-sieve primes
///
SieveOfEratosthenes::SieveOfEratosthenes(uint64_t start,
                                         uint64_t stop,
                                         uint64_t sieveSize,
                                         const PreSieve& preSieve) :
  start_(start),
  stop_(stop),
  sqrtStop_(isqrt(stop)),
  preSieve_(preSieve),
  maxPreSieve_(preSieve.getMaxPrime()),
  sieve_(nullptr)
{
  if (start_ < 7)
    throw primesieve_error("SieveOfEratosthenes: start must be >= 7");
  if (start_ > stop_)
    throw primesieve_error("SieveOfEratosthenes: start must be <= stop");

  sieveSize_ = floorPow2(sieveSize);
  sieveSize_ = inBetween(8, sieveSize_, 4096);
  sieveSize_ *= 1024;

  uint64_t rem = getByteRemainder(start_);
  uint64_t dist = sieveSize_ * NUMBERS_PER_BYTE + 1;
  segmentLow_ = start_ - rem;
  segmentHigh_ = checkedAdd(segmentLow_, dist);

  allocate();
}

void SieveOfEratosthenes::allocate()
{
  deleteSieve_.reset(new byte_t[sieveSize_]);
  sieve_ = deleteSieve_.get();

  uint64_t l1Size = EratSmall::getL1Size(sieveSize_);
  maxEratSmall_  = (uint64_t) (l1Size     * config::FACTOR_ERATSMALL);
  maxEratMedium_ = (uint64_t) (sieveSize_ * config::FACTOR_ERATMEDIUM);

  if (sqrtStop_ > maxPreSieve_)   eratSmall_.reset(new EratSmall (stop_, l1Size, maxEratSmall_));
  if (sqrtStop_ > maxEratSmall_) eratMedium_.reset(new EratMedium(stop_, sieveSize_, maxEratMedium_));
  if (sqrtStop_ > maxEratMedium_)   eratBig_.reset(new EratBig   (stop_, sieveSize_, sqrtStop_));
}

uint64_t SieveOfEratosthenes::getSqrtStop() const
{
  return sqrtStop_;
}

uint64_t SieveOfEratosthenes::getByteRemainder(uint64_t n)
{
  uint64_t r = n % NUMBERS_PER_BYTE;
  if (r <= 1)
    r += NUMBERS_PER_BYTE;
  return r;
}

/// Pre-sieve multiples of small primes e.g. <= 19
/// to speed up the sieve of Eratosthenes
///
void SieveOfEratosthenes::preSieve()
{
  preSieve_.copy(sieve_, sieveSize_, segmentLow_);

  // unset bits < start
  if (segmentLow_ <= start_)
  {
    if (start_ <= maxPreSieve_)
      sieve_[0] = 0xff;
    uint64_t rem = getByteRemainder(start_);
    sieve_[0] &= unsetSmaller[rem];
  }
}

void SieveOfEratosthenes::crossOffMultiples()
{
  if (eratSmall_)   eratSmall_->crossOff(sieve_, sieveSize_);
  if (eratMedium_) eratMedium_->crossOff(sieve_, sieveSize_);
  if (eratBig_)       eratBig_->crossOff(sieve_);
}

void SieveOfEratosthenes::sieveSegment()
{
  preSieve();
  crossOffMultiples();
  generatePrimes(sieve_, sieveSize_);

  // update for next segment
  uint64_t dist = sieveSize_ * NUMBERS_PER_BYTE;
  segmentLow_ = checkedAdd(segmentLow_, dist);
  segmentHigh_ = checkedAdd(segmentHigh_, dist);
}

/// Sieve the remaining segments, most segments
/// are sieved in addSievingPrime()
///
void SieveOfEratosthenes::sieve()
{
  while (segmentHigh_ < stop_)
    sieveSegment();

  uint64_t rem = getByteRemainder(stop_);
  uint64_t dist = (stop_ - rem) - segmentLow_;
  sieveSize_ = dist / NUMBERS_PER_BYTE + 1;
  dist = sieveSize_ * NUMBERS_PER_BYTE + 1;
  segmentHigh_ = checkedAdd(segmentLow_, dist);

  // sieve last segment
  preSieve();
  crossOffMultiples();

  // unset bits > stop
  sieve_[sieveSize_ - 1] &= unsetLarger[rem];

  // unset bytes > stop
  if (sieveSize_ % 8)
    memset(&sieve_[sieveSize_], 0, 8 - sieveSize_ % 8);

  generatePrimes(sieve_, sieveSize_);
}

} // namespace
