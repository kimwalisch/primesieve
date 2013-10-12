///
/// @file   PushBackPrimes.h
/// @brief  This file contains classes needed to store primes in
///         std::vector objects. These classes derive from
///         PrimeSieveCallback and call PrimeSieve's callbackPrimes()
///         method, the primes are then pushed back into the vector
///         inside the callback method.
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PUSHBACKPRIMES_H
#define PUSHBACKPRIMES_H

#include "PrimeSieve.h"
#include "PrimeSieveCallback.h"
#include "cancel_callback.h"

#include <stdint.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace soe {

inline uint64_t approximate_prime_count(uint64_t start, uint64_t stop)
{
  if (start > stop)
    return 0;
  if (stop < 10)
  {
    uint64_t pi[10] = { 0, 0, 1, 2, 2, 3, 3, 4, 4, 4 };
    return (pi[stop] - pi[start]) + 1;
  }
  return static_cast<uint64_t>((stop - start) / (std::log(static_cast<double>(stop)) - 1.1));
}

template <typename T>
class PushBackPrimes : public PrimeSieveCallback<uint64_t>
{
public:
  PushBackPrimes(std::vector<T>& primes)
    : primes_(primes)
  { }
  void pushBackPrimes(uint64_t start, uint64_t stop)
  {
    if (start <= stop)
    {
      uint64_t prime_count = approximate_prime_count(start, stop);
      uint64_t limit = std::numeric_limits<std::size_t>::max();
      primes_.reserve(primes_.size() + static_cast<std::size_t>(std::min(prime_count, limit)));
      PrimeSieve ps;
      ps.callbackPrimes(start, stop, this);
    }
  }
  void callback(uint64_t prime)
  {
    primes_.push_back(static_cast<T>(prime));
  }
private:
  PushBackPrimes(const PushBackPrimes&);
  void operator=(const PushBackPrimes&);
  std::vector<T>& primes_;
};

template <typename T>
class PushBack_N_Primes : public PrimeSieveCallback<uint64_t>
{
public:
  PushBack_N_Primes(std::vector<T>& primes) 
    : primes_(primes)
  { }
  void pushBack_N_Primes(uint64_t n, uint64_t start)
  {
    n_ = n;
    primes_.reserve(primes_.size() + n_);
    PrimeSieve ps;
    try
    {
      while (n_ > 0)
      {
        uint64_t logn = 50;
        // choose stop > nth prime
        uint64_t stop = start + n_ * logn + 10000;
        ps.callbackPrimes(start, stop, this);
        start = stop + 1;
      }
    }
    catch (cancel_callback&) { }
  }
  void callback(uint64_t prime)
  {
    primes_.push_back(static_cast<T>(prime));
    if (--n_ == 0)
      throw cancel_callback();
  }
  private:
    PushBack_N_Primes(const PushBack_N_Primes&);
    void operator=(const PushBack_N_Primes&);
    std::vector<T>& primes_;
    uint64_t n_;
};

} // namespace soe

#endif
