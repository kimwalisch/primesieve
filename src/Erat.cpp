///
/// @file   Erat.cpp
/// @brief  The Erat class manages prime sieving using the
///         EratSmall, EratMedium, EratBig classes.
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/config.hpp>
#include <primesieve/Erat.hpp>
#include <primesieve/EratSmall.hpp>
#include <primesieve/EratMedium.hpp>
#include <primesieve/EratBig.hpp>
#include <primesieve/PreSieve.hpp>
#include <primesieve/pmath.hpp>
#include <primesieve/primesieve_error.hpp>

#include <stdint.h>
#include <cstring>
#include <memory>

using namespace std;
using namespace primesieve;

namespace {

/// unset bits < start
const byte_t unsetSmaller[37] =
{
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xfe, 0xfe, 0xfe, 0xfe, 0xfc, 0xfc, 0xf8, 0xf8,
  0xf8, 0xf8, 0xf0, 0xf0, 0xe0, 0xe0, 0xe0, 0xe0,
  0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0x80, 0x80,
  0x00, 0x00, 0x00, 0x00, 0x00
};

/// unset bits > stop
const byte_t unsetLarger[37] =
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x01, 0x03, 0x03, 0x07, 0x07, 0x07,
  0x07, 0x0f, 0x0f, 0x1f, 0x1f, 0x1f, 0x1f, 0x3f,
  0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x7f, 0x7f, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff
};

} // namespace

namespace primesieve {

/// De Bruijn bitscan table
const uint64_t Erat::bruijnBitValues_[64] =
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

Erat::Erat() { }

Erat::Erat(uint64_t start, uint64_t stop) :
  start_(start),
  stop_(stop)
{ }

Erat::~Erat() { }

/// @start:      Sieve primes >= start
/// @stop:       Sieve primes <= stop
/// @sieveSize:  Sieve size in kilobytes
/// @preSieve:   Pre-sieve primes
///
void Erat::init(uint64_t start,
                uint64_t stop,
                uint64_t sieveSize,
                PreSieve& preSieve)
{
  if (start > stop)
    return;

  if (start < 7)
    throw primesieve_error("Erat: start must be >= 7");

  start_ = start;
  stop_ = stop;
  sqrtStop_ = isqrt(stop);
  preSieve_ = &preSieve;
  maxPreSieve_ = preSieve.getMaxPrime();

  sieveSize_ = floorPow2(sieveSize);
  sieveSize_ = inBetween(8, sieveSize_, 4096);
  sieveSize_ *= 1024;

  uint64_t rem = getByteRemainder(start_);
  uint64_t dist = sieveSize_ * 30 + 6;
  segmentLow_ = start_ - rem;
  segmentHigh_ = checkedAdd(segmentLow_, dist);

  init();
}

void Erat::init()
{
  deleteSieve_.reset(new byte_t[sieveSize_]);
  sieve_ = deleteSieve_.get();

  uint64_t l1Size = EratSmall::getL1Size(sieveSize_);
  maxEratSmall_  = (uint64_t) (l1Size * config::FACTOR_ERATSMALL);
  maxEratMedium_ = (uint64_t) (sieveSize_ * config::FACTOR_ERATMEDIUM);

  if (sqrtStop_ > maxPreSieve_)
    eratSmall_.init(stop_, l1Size, maxEratSmall_);
  if (sqrtStop_ > maxEratSmall_)
    eratMedium_.init(stop_, sieveSize_, maxEratMedium_);
  if (sqrtStop_ > maxEratMedium_)
    eratBig_.init(stop_, sieveSize_, sqrtStop_);
}

bool Erat::hasNextSegment() const
{
  return segmentLow_ < stop_;
}

uint64_t Erat::getByteRemainder(uint64_t n)
{
  n %= 30;
  if (n <= 6) n += 30;
  return n;
}

/// Pre-sieve multiples of small primes e.g. <= 19
/// to speed up the sieve of Eratosthenes
///
void Erat::preSieve()
{
  preSieve_->copy(sieve_, sieveSize_, segmentLow_);

  // unset bits < start
  if (segmentLow_ <= start_)
  {
    if (start_ <= maxPreSieve_)
      sieve_[0] = 0xff;
    uint64_t rem = getByteRemainder(start_);
    sieve_[0] &= unsetSmaller[rem];
  }
}

void Erat::crossOff()
{
  if (eratSmall_.enabled())
    eratSmall_.crossOff(sieve_, sieveSize_);
  if (eratMedium_.enabled())
    eratMedium_.crossOff(sieve_, sieveSize_);
  if (eratBig_.enabled())
    eratBig_.crossOff(sieve_);
}

void Erat::sieve()
{
  while (hasNextSegment())
    sieveSegment();
}

void Erat::sieveSegment()
{
  if (segmentHigh_ >= stop_)
    sieveLastSegment();
  else
  {
    preSieve();
    crossOff();
    generatePrimes(sieve_, sieveSize_);

    uint64_t dist = sieveSize_ * 30;
    segmentLow_ = checkedAdd(segmentLow_, dist);
    segmentHigh_ = checkedAdd(segmentHigh_, dist);
  }
}

void Erat::sieveLastSegment()
{
  uint64_t rem = getByteRemainder(stop_);
  uint64_t dist = (stop_ - rem) - segmentLow_;
  sieveSize_ = dist / 30 + 1;
  dist = sieveSize_ * 30 + 6;
  segmentHigh_ = checkedAdd(segmentLow_, dist);

  preSieve();
  crossOff();

  // unset bits > stop
  sieve_[sieveSize_ - 1] &= unsetLarger[rem];

  // unset bytes > stop
  if (sieveSize_ % 8)
    memset(&sieve_[sieveSize_], 0, 8 - sieveSize_ % 8);

  generatePrimes(sieve_, sieveSize_);
  segmentLow_ = stop_;
  segmentHigh_ = stop_;
}

} // namespace
