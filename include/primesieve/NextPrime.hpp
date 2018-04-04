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
#include "littleendian_cast.hpp"
#include "SievingPrimes.hpp"

#include <stdint.h>
#include <vector>

namespace primesieve {

class NextPrime : public Erat
{
public:
  NextPrime(uint64_t, uint64_t);

  void fill(std::vector<uint64_t>* primes, std::size_t* size)
  {
    fill(primes->data(), size);
  }
private:
  uint64_t low_ = 0;
  uint64_t sieveIdx_ = ~0ull;
  uint64_t sievingPrime_ = 0;
  PreSieve preSieve_;
  SievingPrimes sievingPrimes_;
  bool isInit_ = false;
  void init();
  void initSmallPrimes(uint64_t*, std::size_t*);
  bool sieveSegment(uint64_t*, std::size_t*);

  void fill(uint64_t* primes, std::size_t* size)
  {
    if (sieveIdx_ >= sieveSize_)
      if (!sieveSegment(primes, size))
        return;

    uint64_t i = 0;
    uint64_t bits = littleendian_cast<uint64_t>(&sieve_[sieveIdx_]);
    sieveIdx_ += 8;

    for (; bits != 0; i++)
      primes[i] = getPrime(&bits, low_);

    *size = i;
    low_ += 8 * 30;
  }
};

} // namespace

#endif
