///
/// @file  SievingPrimes.hpp
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef SIEVINGPRIMES_HPP
#define SIEVINGPRIMES_HPP

#include "config.hpp"
#include "Erat.hpp"

#include <vector>

namespace primesieve {

class PrimeGenerator;
class PreSieve;

class SievingPrimes : public Erat
{
public:
  SievingPrimes(PrimeGenerator&, const PreSieve&);
  uint64_t nextPrime();
private:
  uint64_t i_;
  uint64_t num_;
  uint64_t low_;
  uint64_t tinyIdx_;
  uint64_t sieveIdx_;
  uint64_t primes_[64];
  std::vector<char> tinySieve_;
  void tinySieve();
  void fill();
  bool sieveSegment();
};

inline uint64_t SievingPrimes::nextPrime()
{
  while (i_ >= num_)
    fill();

  return primes_[i_++];
}

} // namespace

#endif
