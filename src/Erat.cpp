///
/// @file   Erat.cpp
/// @brief  The Erat class manages prime sieving using the
///         EratSmall, EratMedium, EratBig classes.
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/config.hpp>
#include <primesieve/CpuInfo.hpp>
#include <primesieve/Erat.hpp>
#include <primesieve/EratSmall.hpp>
#include <primesieve/EratMedium.hpp>
#include <primesieve/EratBig.hpp>
#include <primesieve/PreSieve.hpp>
#include <primesieve/pmath.hpp>

#include <stdint.h>
#include <array>
#include <algorithm>
#include <cassert>
#include <memory>

namespace {

/// unset bits < start
const std::array<uint8_t, 37> unsetSmaller =
{
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xfe, 0xfe, 0xfe, 0xfe, 0xfc, 0xfc, 0xf8, 0xf8,
  0xf8, 0xf8, 0xf0, 0xf0, 0xe0, 0xe0, 0xe0, 0xe0,
  0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0x80, 0x80,
  0x00, 0x00, 0x00, 0x00, 0x00
};

/// unset bits > stop
const std::array<uint8_t, 37> unsetLarger =
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x01, 0x03, 0x03, 0x07, 0x07, 0x07,
  0x07, 0x0f, 0x0f, 0x1f, 0x1f, 0x1f, 0x1f, 0x3f,
  0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x7f, 0x7f, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff
};

} // namespace

namespace primesieve {

Erat::Erat() = default;

Erat::Erat(uint64_t start, uint64_t stop) :
  start_(start),
  stop_(stop)
{ }

/// @start:     Sieve primes >= start
/// @stop:      Sieve primes <= stop
/// @sieveSize: Sieve size in KiB
/// @preSieve:  Pre-sieve small primes
///
void Erat::init(uint64_t start,
                uint64_t stop,
                uint64_t sieveSize,
                PreSieve& preSieve)
{
  if (start > stop)
    return;

  assert(start >= 7);
  start_ = start;
  stop_ = stop;
  preSieve_ = &preSieve;
  preSieve_->init(start, stop);
  maxPreSieve_ = preSieve_->getMaxPrime();
  initSieve(sieveSize);

  // The 8 bits of each byte of the sieve array correspond to
  // the offsets { 7, 11, 13, 17, 19, 23, 29, 31 }. If we
  // would set dist = sieveSize * 30 we would not include the
  // last bit of the last byte which corresponds to the offset
  // 31. For this reason we set dist = sieveSize * 30 + 6.
  uint64_t rem = byteRemainder(start);
  uint64_t dist = sieveSize_ * 30 + 6;
  segmentLow_ = start_ - rem;
  segmentHigh_ = checkedAdd(segmentLow_, dist);
  segmentHigh_ = std::min(segmentHigh_, stop);

  initErat();
}

void Erat::initSieve(uint64_t sieveSize)
{
  sieveSize_ = floorPow2(sieveSize);
  sieveSize_ = inBetween(16, sieveSize_, 4096);
  sieveSize_ *= 1024;

  sieve_ = new uint8_t[sieveSize_];
  deleter_.reset(sieve_);
}

void Erat::initErat()
{
  uint64_t sqrtStop = isqrt(stop_);
  uint64_t l1CacheSize = getL1CacheSize();

  maxEratSmall_ = (uint64_t) (l1CacheSize * config::FACTOR_ERATSMALL);
  maxEratMedium_ = (uint64_t) (sieveSize_ * config::FACTOR_ERATMEDIUM);

  if (sqrtStop > maxPreSieve_)
    eratSmall_.init(stop_, l1CacheSize, maxEratSmall_);
  if (sqrtStop > maxEratSmall_)
    eratMedium_.init(stop_, sieveSize_, maxEratMedium_);
  if (sqrtStop > maxEratMedium_)
    eratBig_.init(stop_, sieveSize_, sqrtStop);
}

/// EratMedium and EratBig usually run fastest using a sieve
/// size that is slightly smaller than the CPU's L2 cache size.
/// EratSmall however runs fastest using a sieve size that
/// matches the CPU's L1 cache size. Hence we use a smaller
/// sieve size (L1 cache size) in EratSmall and a larger sieve
/// size (< L2 cache size) in both EratMedium and EratBig.
///
uint64_t Erat::getL1CacheSize() const
{
  uint64_t size = cpuInfo.hasL1Cache() ? cpuInfo.l1CacheBytes() : config::L1D_CACHE_BYTES;
  uint64_t minSize = 8 << 10;
  uint64_t maxSize = 4096 << 10;

  size = std::min(size, sieveSize_);
  size = inBetween(minSize, size, maxSize);

  return size;
}

bool Erat::hasNextSegment() const
{
  return segmentLow_ < stop_;
}

uint64_t Erat::byteRemainder(uint64_t n)
{
  n %= 30;
  if (n <= 6) n += 30;
  return n;
}

/// Pre-sieve multiples of small primes e.g. <= 59
/// to speed up the sieve of Eratosthenes
///
void Erat::preSieve()
{
  preSieve_->preSieve(sieve_, sieveSize_, segmentLow_);

  // unset bits < start
  if (segmentLow_ <= start_)
  {
    uint64_t rem = byteRemainder(start_);
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

void Erat::sieveSegment()
{
  if (segmentHigh_ == stop_)
    sieveLastSegment();
  else
  {
    preSieve();
    crossOff();

    uint64_t dist = sieveSize_ * 30;
    segmentLow_ = checkedAdd(segmentLow_, dist);
    segmentHigh_ = checkedAdd(segmentHigh_, dist);
    segmentHigh_ = std::min(segmentHigh_, stop_);
  }
}

void Erat::sieveLastSegment()
{
  uint64_t rem = byteRemainder(stop_);
  uint64_t dist = (stop_ - rem) - segmentLow_;
  sieveSize_ = dist / 30 + 1;

  preSieve();
  crossOff();

  // unset bits > stop
  sieve_[sieveSize_ - 1] &= unsetLarger[rem];

  // unset bytes > stop
  uint64_t bytes = sieveSize_ % 8;
  bytes = (8 - bytes) % 8;
  std::fill_n(&sieve_[sieveSize_], bytes, (uint8_t) 0);

  segmentLow_ = stop_;
}

} // namespace
