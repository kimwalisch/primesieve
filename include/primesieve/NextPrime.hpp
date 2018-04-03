///
/// @file  NextPrime.hpp
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef NEXTPRIME_HPP
#define NEXTPRIME_HPP

#include "Erat.hpp"
#include "PreSieve.hpp"
#include "SievingPrimes.hpp"

#include <stdint.h>

namespace primesieve {

class NextPrime : public Erat
{
public:
  NextPrime(uint64_t, uint64_t, uint64_t);
  uint64_t nextPrime();
private:
  uint64_t i_ = 0;
  uint64_t num_ = 0;
  uint64_t low_ = 0;
  uint64_t sieveIdx_ = ~0ull;
  uint64_t sievingPrime_ = 0;
  PreSieve preSieve_;
  SievingPrimes sievingPrimes_;
  uint64_t primes_[64];
  void initSmallPrimes(uint64_t, uint64_t);
  void fill();
  bool sieveSegment();
};

inline uint64_t NextPrime::nextPrime()
{
  while (i_ >= num_)
    fill();

  return primes_[i_++];
}

} // namespace

#endif
