///
/// @file  PrimeGenerator.hpp
///        Generates the primes inside [start, stop] and stores them
///        in a vector. After the primes have been stored in the
///        vector primesieve::iterator iterates over the vector and
///        returns the primes. When there are no more primes left in
///        the vector PrimeGenerator generates new primes.
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRIMEGENERATOR_HPP
#define PRIMEGENERATOR_HPP

#include "Erat.hpp"
#include "PreSieve.hpp"
#include "littleendian_cast.hpp"
#include "SievingPrimes.hpp"

#include <stdint.h>
#include <array>
#include <vector>

namespace primesieve {

class PrimeGenerator : public Erat
{
public:
  PrimeGenerator(uint64_t start, uint64_t stop);
  void fill(std::vector<uint64_t>&);

  bool finished() const
  {
    return finished_;
  }

  static uint64_t maxCachedPrime()
  {
    return smallPrimes.back();
  }

  void fill(std::vector<uint64_t>& primes,
            std::size_t* size)
  {
    if (sieveIdx_ >= sieveSize_)
      if (!sieveSegment(primes, size))
        return;

    std::size_t i = 0;
    uint64_t bits = littleendian_cast<uint64_t>(&sieve_[sieveIdx_]);
    sieveIdx_ += 8;

    for (; bits != 0; i++)
      primes[i] = nextPrime(&bits, low_);

    *size = i;
    low_ += 8 * 30;
  }
private:
  uint64_t low_ = 0;
  uint64_t sieveIdx_ = ~0ull;
  uint64_t prime_ = 0;
  PreSieve preSieve_;
  SievingPrimes sievingPrimes_;
  bool isInit_ = false;
  bool finished_ = false;
  static const std::array<uint64_t, 64> smallPrimes;
  static const std::array<uint8_t, 312> primePi;
  std::size_t getStartIdx() const;
  std::size_t getStopIdx() const;
  void initErat();
  void init(std::vector<uint64_t>&);
  void init(std::vector<uint64_t>&, std::size_t*);
  bool sieveSegment(std::vector<uint64_t>&);
  bool sieveSegment(std::vector<uint64_t>&, std::size_t*);
  void sieveSegment();
};

} // namespace

#endif
