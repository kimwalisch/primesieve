///
/// @file   ParallelSieve.hpp
/// @brief  The ParallelSieve class provides an easy API for
///         multi-threaded prime sieving.
///
/// Copyright (C) 2026 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PARALLELSIEVE_HPP
#define PARALLELSIEVE_HPP

#include "PrimeSieveClass.hpp"

#include <primesieve/config.hpp>
#include <primesieve/macros.hpp>

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
  uint64_t getThreadDistance(int) const;
  uint64_t align(uint64_t) const;

  int numThreads_ = 0;
  MAYBE_UNUSED char pad1[config::MAX_CACHE_LINE_SIZE];
  std::mutex mutex_;
  MAYBE_UNUSED char pad2[config::MAX_CACHE_LINE_SIZE];
};

} // namespace

#endif
