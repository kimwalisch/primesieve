///
/// @file   SieveOfEratosthenes.cpp
/// @brief  Implementation of the segmented sieve of Eratosthenes.
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
#include <memory>

namespace primesieve {

const uint_t SieveOfEratosthenes::bitValues_[8] = { 7, 11, 13, 17, 19, 23, 29, 31 };

/// De Bruijn bitscan table
const uint_t SieveOfEratosthenes::bruijnBitValues_[64] =
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
/// @preSieve:   Pre-sieve primes <= preSieve.getLimit()
///
SieveOfEratosthenes::SieveOfEratosthenes(uint64_t start,
                                         uint64_t stop,
                                         uint_t sieveSize,
                                         const PreSieve& preSieve) :
  start_(start),
  stop_(stop),
  preSieve_(preSieve),
  limitPreSieve_(preSieve.getLimit()),
  sieve_(nullptr)
{
  if (start_ < 7)
    throw primesieve_error("SieveOfEratosthenes: start must be >= 7");
  if (start_ > stop_)
    throw primesieve_error("SieveOfEratosthenes: start must be <= stop");

  // sieveSize_ must be a power of 2
  sieveSize_ = inBetween(1, floorPowerOf2(sieveSize), 2048);
  sieveSize_ *= 1024; // convert to bytes

  uint64_t dist = sieveSize_ * NUMBERS_PER_BYTE + 1;
  segmentLow_ = start_ - getByteRemainder(start_);
  segmentHigh_ = checkedAdd(segmentLow_, dist);
  sqrtStop_ = (uint_t) isqrt(stop_);

  allocate();
}

void SieveOfEratosthenes::allocate()
{
  deleteSieve_.reset(new byte_t[sieveSize_]);
  sieve_ = deleteSieve_.get();

  limitEratSmall_  = (uint_t)(sieveSize_ * config::FACTOR_ERATSMALL);
  limitEratMedium_ = (uint_t)(sieveSize_ * config::FACTOR_ERATMEDIUM);

  if (sqrtStop_ > limitPreSieve_)   eratSmall_.reset(new EratSmall (stop_, sieveSize_, limitEratSmall_));
  if (sqrtStop_ > limitEratSmall_) eratMedium_.reset(new EratMedium(stop_, sieveSize_, limitEratMedium_));
  if (sqrtStop_ > limitEratMedium_)   eratBig_.reset(new EratBig   (stop_, sieveSize_, sqrtStop_));
}

uint_t SieveOfEratosthenes::getSqrtStop() const
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

  // unset bits (numbers) < start
  if (segmentLow_ <= start_)
  {
    if (start_ <= limitPreSieve_)
      sieve_[0] = 0xff;
    for (int i = 0; bitValues_[i] < getByteRemainder(start_); i++)
      sieve_[0] &= 0xfe << i;
  }
}

void SieveOfEratosthenes::crossOffMultiples()
{
  if (eratSmall_)   eratSmall_->crossOff(sieve_, &sieve_[sieveSize_]);
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

  uint64_t remainder = getByteRemainder(stop_);
  uint64_t dist = (stop_ - remainder) - segmentLow_;
  sieveSize_ = ((uint_t) dist) / NUMBERS_PER_BYTE + 1;
  dist = sieveSize_ * NUMBERS_PER_BYTE + 1;
  segmentHigh_ = checkedAdd(segmentLow_, dist);

  // sieve the last segment
  preSieve();
  crossOffMultiples();

  int i;
  // unset bits (numbers) > stop_
  for (i = 0; i < 8; i++)
    if (bitValues_[i] > remainder)
      break;
  int unsetBits = ~(0xff << i);
  sieve_[sieveSize_ - 1] &= unsetBits;

  // unset bytes (numbers) > stop_
  for (uint_t j = sieveSize_; j % 8 != 0; j++)
    sieve_[j] = 0;

  generatePrimes(sieve_, sieveSize_);
}

} // namespace
