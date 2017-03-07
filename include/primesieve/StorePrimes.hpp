///
/// @file   StorePrimes.hpp
/// @brief  This file contains classes needed to store primes in
///         std::vector objects. These classes derive from Callback
///         and call PrimeSieve's callbackPrimes() method, the
///         primes are then pushed back onto the vector inside the
///         callback method.
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PUSHBACKPRIMES_HPP
#define PUSHBACKPRIMES_HPP

#include "PrimeSieve.hpp"
#include "Callback.hpp"
#include "primesieve_error.hpp"

#include <stdint.h>
#include <exception>
#include <cmath>

namespace primesieve {

/// approximate_prime_count(x) >= pi(x)
inline std::size_t approximate_prime_count(uint64_t start, uint64_t stop)
{
  if (start > stop)
    return 0;
  if (stop <= 10)
    return 4;

  // pi(x) <= x / (log(x) - 1.1) + 5, for x >= 4
  double x = (double) stop;
  double logx = std::log(x);
  double div = logx - 1.1;
  double pix = (stop - start) / div + 5;

  return (std::size_t) pix;
}

template <typename T>
class StorePrimes : public Callback
{
public:
  StorePrimes(T& primes)
    : primes_(primes)
  { }

  void callback(uint64_t prime)
  {
    typedef typename T::value_type V;
    primes_.push_back((V) prime);
  }

  void storePrimes(uint64_t start, uint64_t stop)
  {
    if (start <= stop)
    {
      std::size_t prime_count = approximate_prime_count(start, stop);
      primes_.reserve(primes_.size() + prime_count);
      PrimeSieve ps;
      ps.callbackPrimes(start, stop, this);
    }
  }
private:
  T& primes_;
};

template <typename T>
class Store_N_Primes : public Callback
{
public:
  Store_N_Primes(T& primes) 
    : primes_(primes)
  { }

  void callback(uint64_t prime)
  {
    typedef typename T::value_type V;
    primes_.push_back((V) prime);
    if (--n_ == 0)
      throw stop_callback();
  }

  void store_N_Primes(uint64_t n, uint64_t start)
  {
    n_ = n;
    PrimeSieve ps;
    std::size_t newSize = primes_.size() + (std::size_t) n_;
    primes_.reserve(newSize);
    try
    {
      while (n_ > 0)
      {
        // choose stop > nth prime
        uint64_t logx = 50;
        uint64_t dist = n_ * logx + 10000;
        uint64_t stop = start + dist;

        // fix integer overflow
        if (stop < start)
          stop = get_max_stop();

        ps.callbackPrimes(start, stop, this);
        start = stop + 1;

        if (stop >= get_max_stop())
          throw primesieve_error("cannot generate primes > 2^64");
      }
    }
    catch (stop_callback&) { }
  }
private:
  class stop_callback : public std::exception { };
  T& primes_;
  uint64_t n_;
};

} // namespace

#endif
