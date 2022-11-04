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
#include <primesieve/macros.hpp>
#include <primesieve/pmath.hpp>

#include <stdint.h>
#include <algorithm>
#include <limits>

namespace {

/// unset bits < start
const primesieve::pod_array<uint8_t, 37> unsetSmaller =
{
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xfe, 0xfe, 0xfe, 0xfe, 0xfc, 0xfc, 0xf8, 0xf8,
  0xf8, 0xf8, 0xf0, 0xf0, 0xe0, 0xe0, 0xe0, 0xe0,
  0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0x80, 0x80,
  0x00, 0x00, 0x00, 0x00, 0x00
};

/// unset bits > stop
const primesieve::pod_array<uint8_t, 37> unsetLarger =
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x01, 0x03, 0x03, 0x07, 0x07, 0x07,
  0x07, 0x0f, 0x0f, 0x1f, 0x1f, 0x1f, 0x1f, 0x3f,
  0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x7f, 0x7f, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff
};

} // namespace

namespace primesieve {

Erat::Erat(uint64_t start, uint64_t stop) :
  start_(start),
  stop_(stop)
{ }

/// @start: Sieve primes >= start.
/// @stop:  Sieve primes <= stop.
/// @maxSieveSize: Maximum sieve array size in kilobytes.
/// @preSieve: Pre-sieve small primes.
///
void Erat::init(uint64_t start,
                uint64_t stop,
                uint64_t maxSieveSize,
                PreSieve& preSieve,
                MemoryPool& memoryPool)
{
  if_unlikely(start > stop || 
              start >= std::numeric_limits<uint64_t>::max())
    return;

  ASSERT(start >= 7);
  ASSERT(maxSieveSize >= 16);
  ASSERT(maxSieveSize <= 8192);

  start_ = start;
  stop_ = stop;
  preSieve_ = &preSieve;
  maxPreSieve_ = preSieve_->getMaxPrime();

  // Convert KiB to bytes
  maxSieveSize <<= 10;
  initAlgorithms(maxSieveSize, memoryPool);
}

/// EratMedium and EratBig usually run fastest using a sieve
/// size that is slightly smaller than the CPU's L2 cache size.
/// EratSmall however runs fastest using a sieve size that
/// matches the CPU's L1 cache size. Hence we use a smaller
/// sieve size (L1 cache size) in EratSmall and a larger sieve
/// size (< L2 cache size) in both EratMedium and EratBig.
///
uint64_t Erat::getL1CacheSize()
{
  if (cpuInfo.hasL1Cache())
    return cpuInfo.l1CacheBytes();
  else
    return config::L1D_CACHE_BYTES;
}

void Erat::initAlgorithms(uint64_t maxSieveSize,
                          MemoryPool& memoryPool)
{
  uint64_t sqrtStop = isqrt(stop_);
  uint64_t l1CacheSize = getL1CacheSize();
  l1CacheSize = inBetween(16 << 10, l1CacheSize, 8192 << 10);

  // ================================================================
  // 1. sieveSize must satisfy: sieveSize % sizeof(uint64_t) == 0
  // ================================================================

  l1CacheSize = ceilDiv(l1CacheSize, sizeof(uint64_t)) * sizeof(uint64_t);
  maxSieveSize = ceilDiv(maxSieveSize, sizeof(uint64_t)) * sizeof(uint64_t);
  uint64_t minSieveSize = std::min(l1CacheSize, maxSieveSize);

  // ================================================================
  // 2. sieveSize = inBetween(minSieveSize, sqrtStop, maxSieveSize)
  // ================================================================

  // For small stop numbers a small sieve array size that
  // matches the CPU's L1 data cache size performs best.
  // For larger stop numbers a sieve array size that is
  // within [L1CacheSize, L2CacheSize] usually performs best.
  uint64_t sieveSize = inBetween(minSieveSize, sqrtStop, maxSieveSize);
  sieveSize = inBetween(16 << 10, sieveSize, 8192 << 10);
  sieveSize = ceilDiv(sieveSize, sizeof(uint64_t)) * sizeof(uint64_t);
  minSieveSize = std::min(l1CacheSize, sieveSize);

  // ================================================================
  // 3. Initialize upper bounds for EratSmall & EratMedium
  // ================================================================

  // Small sieving primes are processed using the EratSmall
  // algorithm, medium sieving primes are processed using
  // the EratMedium algorithm and large sieving primes are
  // processed using the EratBig algorithm.
  maxEratSmall_ = (uint64_t) (minSieveSize * config::FACTOR_ERATSMALL);
  maxEratMedium_ = (uint64_t) (sieveSize * config::FACTOR_ERATMEDIUM);

  // ================================================================
  // 4. EratBig requires a power of 2 sieve size
  // ================================================================

  if (sqrtStop > maxEratMedium_)
  {
    sieveSize = floorPow2(sieveSize);
    minSieveSize = std::min(l1CacheSize, sieveSize);
    maxEratSmall_ = (uint64_t) (minSieveSize * config::FACTOR_ERATSMALL);
    maxEratMedium_ = (uint64_t) (sieveSize * config::FACTOR_ERATMEDIUM);
  }

  // ================================================================
  // 5. Ensure we allocate the smallest possible amount of memory
  // ================================================================

  maxEratSmall_ = std::min(maxEratSmall_, sqrtStop);
  maxEratMedium_ = std::min(maxEratMedium_, sqrtStop);

  // ================================================================
  // 6. Initialize segment bounds
  // ================================================================

  // The 8 bits of each byte of the sieve array correspond to
  // the offsets { 7, 11, 13, 17, 19, 23, 29, 31 }. If we
  // would set dist = sieveSize * 30 we would not include the
  // last bit of the last byte which corresponds to the offset
  // 31. For this reason we set dist = sieveSize * 30 + 6.
  uint64_t rem = byteRemainder(start_);
  uint64_t dist = sieveSize * 30 + 6;
  segmentLow_ = start_ - rem;
  segmentHigh_ = checkedAdd(segmentLow_, dist);
  segmentHigh_ = std::min(segmentHigh_, stop_);

  // ================================================================
  // 7. Use tiny sieveSize if possible
  // ================================================================

  // If we are sieving just a single segment
  // and the EratBig algorithm is not used, then
  // we can allocate a smaller sieve array.
  if (segmentHigh_ >= stop_ &&
      sqrtStop <= maxEratMedium_)
  {
    uint64_t rem = byteRemainder(stop_);
    uint64_t dist = (stop_ - rem) - segmentLow_;
    sieveSize = dist / 30 + 1;
    sieveSize = ceilDiv(sieveSize, sizeof(uint64_t)) * sizeof(uint64_t);
  }

  // ================================================================
  // 8. Finally, initialize EratSmall, EratMedium & EratBig
  // ================================================================

  ASSERT(sieveSize % sizeof(uint64_t) == 0);
  sieve_.resize(sieveSize);

  if (sqrtStop > maxPreSieve_)
    eratSmall_.init(stop_, l1CacheSize, maxEratSmall_);
  if (sqrtStop > maxEratSmall_)
    eratMedium_.init(stop_, maxEratMedium_, memoryPool);
  if (sqrtStop > maxEratMedium_)
    eratBig_.init(stop_, sieve_.size(), sqrtStop, memoryPool);
}

bool Erat::hasNextSegment() const
{
  return segmentLow_ < stop_;
}

uint64_t Erat::byteRemainder(uint64_t n)
{
  // Return n % 30 using equivalence classes 7..36
  // instead of the usual 0..29.
  ASSERT(n >= 7);
  return (n - 7) % 30 + 7;
}

void Erat::sieveSegment()
{
  if (segmentHigh_ < stop_)
  {
    preSieve();
    crossOff();

    uint64_t dist = sieve_.size() * 30;
    segmentLow_ = checkedAdd(segmentLow_, dist);
    segmentHigh_ = checkedAdd(segmentHigh_, dist);
    segmentHigh_ = std::min(segmentHigh_, stop_);
  }
  else
    sieveLastSegment();
}

void Erat::sieveLastSegment()
{
  uint64_t rem = byteRemainder(stop_);
  uint64_t dist = (stop_ - rem) - segmentLow_;
  sieve_.resize(dist / 30 + 1);

  preSieve();
  crossOff();

  // unset bits > stop
  sieve_.back() &= unsetLarger[rem];

  // unset bytes > stop
  auto* sieve = sieve_.data();
  auto i = sieve_.size();
  ASSERT(sieve_.capacity() % sizeof(uint64_t) == 0);
  for (; i % sizeof(uint64_t); i++)
    sieve[i] = 0;

  segmentLow_ = stop_;
}

/// Pre-sieve multiples of small primes < 100
/// to speed up the sieve of Eratosthenes
///
void Erat::preSieve()
{
  preSieve_->preSieve(sieve_, segmentLow_);

  // unset bits < start
  if (segmentLow_ <= start_)
  {
    uint64_t rem = byteRemainder(start_);
    sieve_[0] &= unsetSmaller[rem];
  }
}

void Erat::crossOff()
{
  if (eratSmall_.hasSievingPrimes())
    eratSmall_.crossOff(sieve_);
  if (eratMedium_.hasSievingPrimes())
    eratMedium_.crossOff(sieve_);
  if (eratBig_.hasSievingPrimes())
    eratBig_.crossOff(sieve_);
}

} // namespace
