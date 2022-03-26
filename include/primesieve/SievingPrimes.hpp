///
/// @file  SievingPrimes.hpp
///
/// Copyright (C) 2019 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef SIEVINGPRIMES_HPP
#define SIEVINGPRIMES_HPP

#include "Erat.hpp"
#include "macros.hpp"

#include <cstdint>
#include <array>
#include <vector>

namespace primesieve {

class PreSieve;

class SievingPrimes : public Erat
{
public:
  SievingPrimes() = default;
  SievingPrimes(Erat*, PreSieve&);
  void init(Erat*, PreSieve&);
  uint64_t next();
private:
  uint64_t i_ = 0;
  uint64_t size_ = 0;
  uint64_t low_ = 0;
  uint64_t tinyIdx_ = 0;
  uint64_t sieveIdx_ = ~0ull;
  std::array<uint64_t, 128> primes_;
  std::vector<char> tinySieve_;
  NOINLINE void fill();
  void tinySieve();
  bool sieveSegment();
};

inline uint64_t SievingPrimes::next()
{
  while (i_ >= size_)
    fill();

  return primes_[i_++];
}

} // namespace

#endif
