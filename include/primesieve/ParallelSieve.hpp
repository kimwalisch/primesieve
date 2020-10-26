///
/// @file   ParallelSieve.hpp
/// @brief  The ParallelSieve class provides an easy API for
///         multi-threaded prime sieving.
///
/// Copyright (C) 2019 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PARALLELSIEVE_HPP
#define PARALLELSIEVE_HPP

#include "PrimeSieve.hpp"
#include <stdint.h>
#include <mutex>

namespace primesieve {

class ParallelSieve : public PrimeSieve
{
public:
  using PrimeSieve::sieve;

  ParallelSieve();
  static int getMaxThreads();
  int getNumThreads() const;
  int idealNumThreads() const;
  void setNumThreads(int numThreads);
  bool tryUpdateStatus(uint64_t);
  virtual void sieve();

private:
  std::mutex mutex_;
  int numThreads_ = 0;
  uint64_t getThreadDistance(int) const;
  uint64_t align(uint64_t) const;
};

} // namespace

#endif
