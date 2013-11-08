///
/// @file   PrimeFinder.cpp
/// @brief  Callback, print and count primes and prime k-tuplets
///         (twin primes, prime triplets, ...).
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/config.hpp>
#include <primesieve/callback_t.hpp>
#include <primesieve/PrimeFinder.hpp>
#include <primesieve/SieveOfEratosthenes.hpp>
#include <primesieve/SieveOfEratosthenes-CALLBACK.hpp>
#include <primesieve/SieveOfEratosthenes-inline.hpp>
#include <primesieve/PrimeSieve.hpp>
#include <primesieve/PrimeSieveCallback.hpp>
#include <primesieve/PrimeSieve-lock.hpp>

#include <stdint.h>
#include <algorithm>
#include <vector>
#include <iostream>
#include <sstream>

namespace primesieve {

/// forward declaration
uint64_t popcount(const uint64_t* array, uint64_t size);

const uint_t PrimeFinder::kBitmasks_[7][5] =
{
  { END },
  { 0x06, 0x18, 0xc0, END },       // Twin prime       bitmasks, i.e. b00000110, b00011000, b11000000
  { 0x07, 0x0e, 0x1c, 0x38, END }, // Prime triplet    bitmasks, i.e. b00000111, b00001110, ...
  { 0x1e, END },                   // Prime quadruplet bitmasks
  { 0x1f, 0x3e, END },             // Prime quintuplet bitmasks
  { 0x3f, END },                   // Prime sextuplet  bitmasks
  { 0xfe, END }                    // Prime septuplet  bitmasks
};

PrimeFinder::PrimeFinder(PrimeSieve& ps) :
  SieveOfEratosthenes(std::max<uint64_t>(7, ps.getStart()),
                      ps.getStop(),
                      ps.getSieveSize()),
  ps_(ps),
  counts_(ps.counts_),
  threadNum_(ps.threadNum_),
  callback_(ps.callback_),
  callback_tn_(ps.callback_tn_),
  psc_(ps.psc_),
  psc_tn_(ps.psc_tn_)
{
  if (ps_.isFlag(ps_.COUNT_TWINS, ps_.COUNT_SEPTUPLETS))
    init_kCounts();
}

/// Calculate the number of twins, triplets, ... (bitmask matches)
/// for each possible byte value 0 - 255.
///
void PrimeFinder::init_kCounts()
{
  for (uint_t i = 1; i < counts_.size(); i++) {
    if (ps_.isCount(i)) {
      kCounts_[i].resize(256);
      for (uint_t j = 0; j < kCounts_[i].size(); j++) {
        uint_t bitmaskCount = 0;
        for (const uint_t* b = kBitmasks_[i]; *b <= j; b++) {
          if ((j & *b) == *b)
            bitmaskCount++;
        }
        kCounts_[i][j] = bitmaskCount;
      }
    }
  }
}

/// Executed after each sieved segment.
/// @see sieveSegment() in SieveOfEratosthenes.cpp
///
void PrimeFinder::segmentFinished(const byte_t* sieve, uint_t sieveSize)
{
  if (ps_.isCallback())
    callback(sieve, sieveSize);
  if (ps_.isCount())
    count(sieve, sieveSize);
  if (ps_.isPrint())
    print(sieve, sieveSize);
  if (ps_.isStatus())
    ps_.updateStatus(sieveSize * NUMBERS_PER_BYTE, /* waitForLock = */ false);
}

/// Count the primes and prime k-tuplets within
/// the current segment.
///
void PrimeFinder::count(const byte_t* sieve, uint_t sieveSize)
{
  // count prime numbers (1 bits), see popcount.cpp
  if (ps_.isFlag(ps_.COUNT_PRIMES))
    counts_[0] += popcount(reinterpret_cast<const uint64_t*>(sieve), (sieveSize + 7) / 8);

  // count prime k-tuplets (i = 1 twins, i = 2 triplets, ...)
  for (uint_t i = 1; i < counts_.size(); i++) {
    if (ps_.isCount(i)) {
      uint_t sum0 = 0;
      uint_t sum1 = 0;
      uint_t sum2 = 0;
      uint_t sum3 = 0;
      for (uint_t j = 0; j < sieveSize; j += 4) {
        sum0 += kCounts_[i][sieve[j+0]];
        sum1 += kCounts_[i][sieve[j+1]];
        sum2 += kCounts_[i][sieve[j+2]];
        sum3 += kCounts_[i][sieve[j+3]];
      }
      counts_[i] += (sum0 + sum1) + (sum2 + sum3);
    }
  }
}

/// Print primes and prime k-tuplets to cout.
/// @note primes < 7 are handled in PrimeSieve::doSmallPrime()
///
void PrimeFinder::print(const byte_t* sieve, uint_t sieveSize) const
{
  if (ps_.isFlag(ps_.PRINT_PRIMES)) {
    LockGuard lock(ps_);
    CALLBACK_PRIMES(printPrime, uint64_t)
  }
  // print prime k-tuplets
  if (ps_.isFlag(ps_.PRINT_TWINS, ps_.PRINT_SEPTUPLETS)) {
    uint_t i = 1; // i = 1 twins, i = 2 triplets, ...
    for (; !ps_.isPrint(i); i++)
      ;
    for (uint_t j = 0; j < sieveSize; j++) {
      for (const uint_t* bitmask = kBitmasks_[i]; *bitmask <= sieve[j]; bitmask++) {
        if ((sieve[j] & *bitmask) == *bitmask) {
          std::ostringstream kTuplet;
          kTuplet << "(";
          uint64_t bits = *bitmask;
          while (bits != 0) {
            kTuplet << getNextPrime(&bits, j);
            kTuplet << ((bits != 0) ? ", " : ")\n");
          }
          std::cout << kTuplet.str();
        }
      }
    }
  }
}

/// Callback the primes within the current segment.
/// @note primes < 7 are handled in PrimeSieve::doSmallPrime()
///
void PrimeFinder::callback(const byte_t* sieve, uint_t sieveSize) const
{
  c_callback_t c_callback = reinterpret_cast<c_callback_t>(callback_);

  if (ps_.isFlag(ps_.PSC_CALLBACK))    { LockGuard lock(ps_); CALLBACK_PRIMES(psc_->callback,  uint64_t) }
  if (ps_.isFlag(ps_.PSC_CALLBACK_TN)) { /* No Locking */     CALLBACK_PRIMES(psc_callback_tn, uint64_t) }
  if (ps_.isFlag(ps_.CALLBACK))        { LockGuard lock(ps_); CALLBACK_PRIMES(callback_,       uint64_t) }
  if (ps_.isFlag(ps_.CALLBACK_TN))     { /* No Locking */     CALLBACK_PRIMES(callback_tn,     uint64_t) }
  if (ps_.isFlag(ps_.C_CALLBACK))      { LockGuard lock(ps_); CALLBACK_PRIMES(c_callback,      uint64_t) }
  if (ps_.isFlag(ps_.C_CALLBACK_TN))   { /* No Locking */     CALLBACK_PRIMES(c_callback_tn,   uint64_t) }
}

void PrimeFinder::printPrime(uint64_t prime)
{
  std::cout << prime << '\n';
}

void PrimeFinder::callback_tn(uint64_t prime) const
{
  callback_tn_(prime, threadNum_);
}

void PrimeFinder::psc_callback_tn(uint64_t prime) const
{
  psc_tn_->callback(prime, threadNum_);
}

void PrimeFinder::c_callback_tn(uint64_t prime) const
{
  c_callback_tn_t c_callback_tn2 = reinterpret_cast<c_callback_tn_t>(callback_tn_);
  c_callback_tn2(prime, threadNum_);
}

} // namespace primesieve
