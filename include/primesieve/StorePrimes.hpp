///
/// @file   StorePrimes.hpp
/// @brief  The StorePrimes and Store_N_Primes classes are used to
///         store primes in std::vector objects. These classes
///         implement a callback function which is called
///         consecutively for all primes in [start, stop].
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef STOREPRIMES_HPP
#define STOREPRIMES_HPP

#include "PrimeSieve.hpp"
#include "primesieve_error.hpp"

#include <stdint.h>
#include <exception>
#include <cmath>

namespace primesieve {

uint64_t get_max_stop();

/// prime_count_approx(x) >= pi(x)
inline std::size_t prime_count_approx(uint64_t start, uint64_t stop)
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

class Store
{
public:
  virtual void operator()(uint64_t prime) = 0;
  virtual ~Store() { }
};

template <typename T>
class StorePrimes : public Store
{
public:
  StorePrimes(T& primes)
    : primes_(primes)
  { }

  // callback function
  void operator()(uint64_t prime)
  {
    primes_.push_back((typename T::value_type) prime);
  }

  void storePrimes(uint64_t start, uint64_t stop)
  {
    if (start <= stop)
    {
      std::size_t size = primes_.size() + prime_count_approx(start, stop);
      primes_.reserve(size);
      PrimeSieve ps;
      ps.storePrimes(start, stop, this);
    }
  }
private:
  T& primes_;
};

template <typename T>
class Store_N_Primes : public Store
{
public:
  Store_N_Primes(T& primes) 
    : primes_(primes)
  { }

  // callback function
  void operator()(uint64_t prime)
  {
    primes_.push_back((typename T::value_type) prime);
    if (--n_ == 0)
      throw stop_store();
  }

  void storePrimes(uint64_t n, uint64_t start)
  {
    n_ = n;
    PrimeSieve ps;
    std::size_t size = primes_.size() + (std::size_t) n_;
    primes_.reserve(size);
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

        ps.storePrimes(start, stop, this);
        start = stop + 1;

        if (stop >= get_max_stop())
          throw primesieve_error("cannot generate primes > 2^64");
      }
    }
    catch (stop_store&) { }
  }
private:
  class stop_store : public std::exception { };
  T& primes_;
  uint64_t n_;
};

} // namespace

#endif
